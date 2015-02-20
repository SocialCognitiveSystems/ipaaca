# -*- coding: utf-8 -*-

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".
#
# Copyright (c) 2009-2014 Social Cognitive Systems Group
#                         CITEC, Bielefeld University
#
# http://opensource.cit-ec.de/projects/ipaaca/
# http://purl.org/net/ipaaca
#
# This file may be licensed under the terms of of the
# GNU Lesser General Public License Version 3 (the ``LGPL''),
# or (at your option) any later version.
#
# Software distributed under the License is distributed
# on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the LGPL for the specific language
# governing rights and limitations.
#
# You should have received a copy of the LGPL along with this
# program. If not, go to http://www.gnu.org/licenses/lgpl.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The development of this software was supported by the
# Excellence Cluster EXC 277 Cognitive Interaction Technology.
# The Excellence Cluster EXC 277 is a grant of the Deutsche
# Forschungsgemeinschaft (DFG) in the context of the German
# Excellence Initiative.

from __future__ import division, print_function


import threading
import uuid

import rsb

import ipaaca_pb2
import ipaaca.defaults
import ipaaca.exception
import ipaaca.converter
import ipaaca.iu



__all__ = [
	'InputBuffer',
	'OutputBuffer',
]

LOGGER = ipaaca.misc.get_library_logger()

class IUStore(dict):
	"""A dictionary storing IUs."""
	def __init__(self):
		super(IUStore, self).__init__()


class FrozenIUStore(IUStore):
	"""A read-only version of a dictionary storing IUs. (TODO: might be slow)"""
	def __init__(self, original_iu_store):
		super(FrozenIUStore, self).__init__()
		map(lambda p: super(FrozenIUStore, self).__setitem__(p[0], p[1]), original_iu_store.items())
	def __delitem__(self, k):
		raise AttributeError()
	def __setitem__(self, k, v):
		raise AttributeError()


class IUEventHandler(object):

	"""Wrapper for IU event handling functions."""

	def __init__(self, handler_function, for_event_types=None, for_categories=None):
		"""Create an IUEventHandler.

		Keyword arguments:
		handler_function -- the handler function with the signature
			(IU, event_type, local)
		for_event_types -- a list of event types or None if handler should
			be called for all event types
		for_categories -- a list of category names or None if handler should
			be called for all categoires
		"""
		super(IUEventHandler, self).__init__()
		self._handler_function = handler_function
		self._for_event_types = (
			None if for_event_types is None else
			(for_event_types[:] if hasattr(for_event_types, '__iter__') else [for_event_types]))
		self._for_categories = (
			None if for_categories is None else
			(for_categories[:] if hasattr(for_categories, '__iter__') else [for_categories]))

	def condition_met(self, event_type, category):
		"""Check whether this IUEventHandler should be called.

		Keyword arguments:
		event_type -- type of the IU event
		category -- category of the IU which triggered the event
		"""
		type_condition_met = (self._for_event_types is None or event_type in self._for_event_types)
		cat_condition_met = (self._for_categories is None or category in self._for_categories)
		return type_condition_met and cat_condition_met

	def call(self, buffer, iu_uid, local, event_type, category):
		"""Call this IUEventHandler's function, if it applies.

		Keyword arguments:
		buffer -- the buffer in which the IU is stored
		iu_uid -- the uid of the IU
		local -- is the IU local or remote to this component? @RAMIN: Is this correct?
		event_type -- IU event type
		category -- category of the IU
		"""
		if self.condition_met(event_type, category):
			iu = buffer._iu_store[iu_uid]
			self._handler_function(iu, event_type, local)


class Buffer(object):

	"""Base class for InputBuffer and OutputBuffer."""

	def __init__(self, owning_component_name, channel=None, participant_config=None):
		'''Create a Buffer.

		Keyword arguments:
		owning_compontent_name -- name of the entity that owns this Buffer
		participant_config -- RSB configuration
		'''
		super(Buffer, self).__init__()
		self._owning_component_name = owning_component_name
		self._channel = channel if channel is not None else ipaaca.defaults.IPAACA_DEFAULT_CHANNEL
		self._participant_config = rsb.ParticipantConfig.fromDefaultSources() if participant_config is None else participant_config
		self._uuid = str(uuid.uuid4())[0:8]
		# Initialise with a temporary, but already unique, name
		self._unique_name = "undef-"+self._uuid
		self._iu_store = IUStore()
		self._iu_event_handlers = []

	def _get_frozen_iu_store(self):
		return FrozenIUStore(original_iu_store = self._iu_store)
	iu_store = property(fget=_get_frozen_iu_store, doc='Copy-on-read version of the internal IU store')

	def register_handler(self, handler_function, for_event_types=None, for_categories=None):
		"""Register a new IU event handler function.

		Keyword arguments:
		handler_function -- a function with the signature (IU, event_type, local)
		for_event_types -- a list of event types or None if handler should
			be called for all event types
		for_categories -- a list of category names or None if handler should
			be called for all categories
		"""
		if handler_function in [h._handler_function for h in self._iu_event_handlers]:
			LOGGER.warn("The handler function '" + handler_function.__name__ + '" has been registered before.')
		handler = IUEventHandler(handler_function=handler_function, for_event_types=for_event_types, for_categories=for_categories)
		self._iu_event_handlers.append(handler)
		return handler

	def call_iu_event_handlers(self, uid, local, event_type, category):
		"""Call registered IU event handler functions registered for this event_type and category."""
		for h in self._iu_event_handlers:
			try:
				h.call(self, uid, local=local, event_type=event_type, category=category)
			except Exception as e:
				if local:
					LOGGER.error('Local IU handler raised an exception upon remote write.' + unicode(e))
				else:
					raise e
	
	def _get_owning_component_name(self):
		"""Return the name of this Buffer's owning component"""
		return self._owning_component_name
	owning_component_name = property(_get_owning_component_name)

	def _get_unique_name(self):
		"""Return the Buffer's unique name."""
		return self._unique_name
	unique_name = property(_get_unique_name)


class InputBuffer(Buffer):

	"""An InputBuffer that holds remote IUs."""

	def __init__(self, owning_component_name, category_interests=None, channel=None, participant_config=None, resend_active=False):
		'''Create an InputBuffer.

		Keyword arguments:
		owning_compontent_name -- name of the entity that owns this InputBuffer
		category_interests -- list of IU categories this Buffer is interested in
		participant_config = RSB configuration
		'''
		super(InputBuffer, self).__init__(owning_component_name, channel, participant_config)
		self._unique_name = '/ipaaca/component/'+str(owning_component_name)+'ID'+self._uuid+'/IB'
		self._resend_active = resend_active
		self._listener_store = {} # one per IU category
		self._remote_server_store = {} # one per remote-IU-owning Component
		self._category_interests = []
		# add own uuid as identifier for hidden category.
		self._add_category_listener(str(self._uuid))
		if category_interests is not None:
			self.add_category_interests(category_interests)

	def _get_remote_server(self, iu):
		'''Return (or create, store and return) a remote server.'''
		_owner = None
		if hasattr(iu,'owner_name'):
			_owner = iu.owner_name
		elif hasattr(iu,'writer_name'):
			_owner = iu.writer_name
		if _owner is not None:
			if _owner in self._remote_server_store:
				return self._remote_server_store[_owner]
			#  TODO remove the str() when unicode is supported (issue #490)
			remote_server = rsb.createRemoteServer(rsb.Scope(str(_owner)))
			self._remote_server_store[_owner] = remote_server
		return remote_server

	def _add_category_listener(self, iu_category):
		'''Create and store a listener on a specific category.'''
		if iu_category not in self._listener_store:
			cat_listener = rsb.createListener(rsb.Scope("/ipaaca/channel/"+str(self._channel)+"/category/"+str(iu_category)), config=self._participant_config)
			cat_listener.addHandler(self._handle_iu_events)
			self._listener_store[iu_category] = cat_listener
			self._category_interests.append(iu_category)
			LOGGER.info("Added listener in scope /ipaaca/channel/" + str(self._channel) + "/category/" + iu_category)

	def _remove_category_listener(self, iu_category):
		'''Remove the listener for a specific category.'''
		if iu_category in self._listener_store and iu_category in self._category_interests:
			del self._listener_store[iu_category]
			self._category_interests.remove(iu_category)
			LOGGER.info("Removed listener in scope /ipaaca/channel/" + str(self._channel) + "/category/ " + iu_category)

	def _handle_iu_events(self, event):
		'''Dispatch incoming IU events.

		Adds incoming IU's to the store, applies payload and commit updates to
		IU, calls IU event handlers.'

		Keyword arguments:
		event -- a converted RSB event
		'''
		type_ = type(event.data)
		if type_ is ipaaca.iu.RemotePushIU:
			# a new IU
			if event.data.uid not in self._iu_store:
				self._iu_store[event.data.uid] = event.data
				event.data.buffer = self
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.ADDED, category=event.data.category)
			else:
				# IU already in our store, overwrite local IU, but do not call
				# event handler. This functionality is necessary to undo 
				# destructive changes after a failing remote updates (undo is
				# done via the resend request mechanism).
				self._iu_store[event.data.uid] = event.data
				event.data.buffer = self
		elif type_ is ipaaca.iu.RemoteMessage:
			# a new Message, an ephemeral IU that is removed after calling handlers
			self._iu_store[ event.data.uid ] = event.data
			event.data.buffer = self
			self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.MESSAGE, category=event.data.category)
			del self._iu_store[ event.data.uid ]
		else:
			if event.data.uid not in self._iu_store:
				if self._resend_active:
					# send resend request to remote server
					self._request_remote_resend(event.data)
				else:
					LOGGER.warning("Received an update for an IU which we did not receive before.")
				return
			# an update to an existing IU
			if type_ is ipaaca_pb2.IURetraction:
				# IU retraction (cannot be triggered remotely)
				iu = self._iu_store[event.data.uid]
				iu._revision = event.data.revision
				iu._apply_retraction() # for now - just sets the _rectracted flag.
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.RETRACTED, category=iu.category)
				# SPECIAL CASE: allow the handlers (which will need to find the IU
				#  in the buffer) to operate on the IU - then delete it afterwards!
				# FIXME: for now: retracted == deleted! Think about this later
				del(self._iu_store[iu.uid])
			else:
				if event.data.writer_name == self.unique_name:
					# Notify only for remotely triggered events;
					# Discard updates that originate from this buffer
					return
				if type_ is ipaaca_pb2.IUCommission:
					# IU commit
					iu = self._iu_store[event.data.uid]
					iu._apply_commission()
					iu._revision = event.data.revision
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.COMMITTED, category=iu.category)
				elif type_ is ipaaca.converter.IUPayloadUpdate:
					# IU payload update
					iu = self._iu_store[event.data.uid]
					iu._apply_update(event.data)
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.UPDATED, category=iu.category)
				elif type_ is ipaaca.converter.IULinkUpdate:
					# IU link update
					iu = self._iu_store[event.data.uid]
					iu._apply_link_update(event.data)
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=ipaaca.iu.IUEventType.LINKSUPDATED, category=iu.category)
				else:
					LOGGER.warning('Warning: _handle_iu_events failed to handle an object of type '+str(type_))

	def add_category_interests(self, category_interests):
		if hasattr(category_interests, '__iter__'):
			for interest in category_interests:
				self._add_category_listener(interest)
		else:
			self._add_category_listener(category_interests)

	def remove_category_interests(self, category_interests):
		if hasattr(category_interests, '__iter__'):
			for interest in category_interests:
				self._remove_category_listener(interest)
		else:
			self._remove_category_listener(category_interests)

	def _request_remote_resend(self, iu):
		remote_server = self._get_remote_server(iu)
		resend_request = ipaaca_pb2.IUResendRequest()
		resend_request.uid = iu.uid # target iu
		resend_request.hidden_scope_name = str(self._uuid) # hidden category name
		remote_revision = remote_server.requestResend(resend_request)
		if remote_revision == 0:
			raise ipaaca.exception.IUResendRequestFailedError()

	def register_handler(self, handler_function, for_event_types=None, for_categories=None):
		"""Register a new IU event handler function.

		Keyword arguments:
		handler_function -- a function with the signature (IU, event_type, local)
		for_event_types -- a list of event types or None if handler should
			be called for all event types
		for_categories -- a list of category names or None if handler should
			be called for all categories
		"""
		handler = super(InputBuffer, self).register_handler(handler_function, for_event_types, for_categories)
		try:
			for category in handler._for_categories:
				self.add_category_interests(category)
		except TypeError:
			# i.e., None was provided to the handler
			pass
		return handler

	def is_resend_active(self):
		return self._resend_active

	def set_resend_active(self, active=True):
		self._resend_active = active


class OutputBuffer(Buffer):

	"""An OutputBuffer that holds local IUs."""

	def __init__(self, owning_component_name, channel=None, participant_config=None):
		'''Create an Output Buffer.

		Keyword arguments:
		owning_component_name -- name of the entity that own this buffer
		participant_config -- RSB configuration
		'''
		super(OutputBuffer, self).__init__(owning_component_name, channel, participant_config)
		self._unique_name = '/ipaaca/component/' + str(owning_component_name) + 'ID' + self._uuid + '/OB'
		self._server = rsb.createServer(rsb.Scope(self._unique_name))
		self._server.addMethod('updateLinks', self._remote_update_links, ipaaca.converter.IULinkUpdate, int)
		self._server.addMethod('updatePayload', self._remote_update_payload, ipaaca.converter.IUPayloadUpdate, int)
		self._server.addMethod('commit', self._remote_commit, ipaaca_pb2.IUCommission, int)
		self._server.addMethod('requestResend', self._remote_request_resend, ipaaca_pb2.IUResendRequest, int)
		self._informer_store = {}
		self._id_prefix = str(owning_component_name)+'-'+str(self._uuid)+'-IU-'
		self.__iu_id_counter_lock = threading.Lock()

	def _remote_update_links(self, update):
		'''Apply a remotely requested update to one of the stored IU's links.'''
		if update.uid not in self._iu_store:
			LOGGER.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
			return 0
		iu = self._iu_store[update.uid]
		with iu.revision_lock:
			if (update.revision != 0) and (update.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				LOGGER.warning("Remote write operation failed because request was out of date; IU "+str(update.uid))
				return 0
			if update.is_delta:
				iu.modify_links(add=update.new_links, remove=update.links_to_remove, writer_name=update.writer_name)
			else:
				iu.set_links(links=update.new_links, writer_name=update.writer_name)
			self.call_iu_event_handlers(update.uid, local=True, event_type=ipaaca.iu.IUEventType.LINKSUPDATED, category=iu.category)
			return iu.revision

	def _remote_update_payload(self, update):
		'''Apply a remotely requested update to one of the stored IU's payload.'''
		if update.uid not in self._iu_store:
			LOGGER.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
			return 0
		iu = self._iu_store[update.uid]
		with iu.revision_lock:
			if (update.revision != 0) and (update.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				LOGGER.warning(u"Remote update_payload operation failed because request was out of date; IU "+str(update.uid))
				LOGGER.warning(u"  Writer was: "+update.writer_name)
				LOGGER.warning(u"  Requested update was: (New keys:) "+','.join(update.new_items.keys())+'  (Removed keys:) '+','.join(update.keys_to_remove))
				LOGGER.warning(u"  Referred-to revision was "+str(update.revision)+' while local revision is '+str(iu.revision))
				return 0
			if update.is_delta:
				#print('Writing delta update by '+str(update.writer_name))
				with iu.payload:
					for k in update.keys_to_remove:
						iu.payload.__delitem__(k, writer_name=update.writer_name)
					for k,v in update.new_items.items():
						iu.payload.__setitem__(k, v, writer_name=update.writer_name)
			else:
				#print('Writing non-incr update by '+str(update.writer_name))
				iu._set_payload(update.new_items, writer_name=update.writer_name)
				# _set_payload etc. have also incremented the revision number
			self.call_iu_event_handlers(update.uid, local=True, event_type=ipaaca.iu.IUEventType.UPDATED, category=iu.category)
			return iu.revision

	def _remote_request_resend(self, iu_resend_request_pack):
		''' Resend a requested IU over the specific hidden category.'''
		if iu_resend_request_pack.uid not in self._iu_store:
			LOGGER.warning("Remote side requested resending of non-existent IU "+str(iu_resend_request_pack.uid))
			return 0
		iu = self._iu_store[iu_resend_request_pack.uid]
		with iu.revision_lock:
			if iu_resend_request_pack.hidden_scope_name is not None and iu_resend_request_pack.hidden_scope_name is not '':
				informer = self._get_informer(iu_resend_request_pack.hidden_scope_name)
				informer.publishData(iu)
				return iu.revision
			else:
				return 0

	def _remote_commit(self, iu_commission):
		'''Apply a remotely requested commit to one of the stored IUs.'''
		if iu_commission.uid not in self._iu_store:
			LOGGER.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(iu_commission.uid))
			return 0
		iu = self._iu_store[iu_commission.uid]
		with iu.revision_lock:
			if (iu_commission.revision != 0) and (iu_commission.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				LOGGER.warning("Remote write operation failed because request was out of date; IU "+str(iu_commission.uid))
				return 0
			if iu.committed:
				return 0
			else:
				iu._internal_commit(writer_name=iu_commission.writer_name)
				self.call_iu_event_handlers(iu_commission.uid, local=True, event_type=ipaaca.iu.IUEventType.COMMITTED, category=iu.category)
				return iu.revision

	def _get_informer(self, iu_category):
		'''Return (or create, store and return) an informer object for IUs of the specified category.'''
		if iu_category in self._informer_store:
			LOGGER.info("Returning informer on scope "+"/ipaaca/channel/"+str(self._channel)+"/category/"+str(iu_category))
			return self._informer_store[iu_category]
		informer_iu = rsb.createInformer(
				rsb.Scope("/ipaaca/channel/"+str(self._channel)+"/category/"+str(iu_category)),
				config=self._participant_config,
				dataType=object)
		self._informer_store[iu_category] = informer_iu #new_tuple
		LOGGER.info("Returning NEW informer on scope "+"/ipaaca/channel/"+str(self._channel)+"/category/"+str(iu_category))
		return informer_iu #return new_tuple

	def add(self, iu):
		'''Add an IU to the IU store, assign an ID and publish it.'''
		if iu.uid in self._iu_store:
			raise ipaaca.exception.IUPublishedError(iu)
		if iu.buffer is not None:
			raise ipaaca.exception.IUPublishedError(iu)
		if iu.access_mode != ipaaca.iu.IUAccessMode.MESSAGE:
			# Messages are not really stored in the OutputBuffer
			self._iu_store[iu.uid] = iu
		iu.buffer = self
		self._publish_iu(iu)

	def remove(self, iu=None, iu_uid=None):
		'''Remove the iu or an IU corresponding to iu_uid from the OutputBuffer, retracting it from the system.'''
		if iu is None:
			if iu_uid is None:
				return None
			else:
				if iu_uid not in self. _iu_store:
					raise ipaaca.exception.IUNotFoundError(iu_uid)
				iu = self._iu_store[iu_uid]
		# unpublish the IU
		self._retract_iu(iu)
		del self._iu_store[iu.uid]
		return iu

	def _publish_iu(self, iu):
		'''Publish an IU.'''
		informer = self._get_informer(iu._category)
		informer.publishData(iu)

	def _retract_iu(self, iu):
		'''Retract (unpublish) an IU.'''
		iu_retraction = ipaaca_pb2.IURetraction()
		iu_retraction.uid = iu.uid
		iu_retraction.revision = iu.revision
		informer = self._get_informer(iu._category)
		informer.publishData(iu_retraction)

	def _send_iu_commission(self, iu, writer_name):
		'''Send IU commission.

		Keyword arguments:
		iu -- the IU that has been committed to
		writer_name -- name of the Buffer that initiated this commit, necessary
			to enable remote components to filter out updates that originated
			from their own operations
		'''
		# a raw Protobuf object for IUCommission is produced
		# (unlike updates, where we have an intermediate class)
		iu_commission = ipaaca_pb2.IUCommission()
		iu_commission.uid = iu.uid
		iu_commission.revision = iu.revision
		iu_commission.writer_name = iu.owner_name if writer_name is None else writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(iu_commission)

	def _send_iu_link_update(self, iu, is_delta, revision, new_links=None, links_to_remove=None, writer_name="undef"):
		'''Send an IU link update.

		Keyword arguments:
		iu -- the IU being updated
		is_delta -- whether this is an incremental update or a replacement
			the whole link dictionary
		revision -- the new revision number
		new_links -- a dictionary of new link sets
		links_to_remove -- a dict of the link sets that shall be removed
		writer_name -- name of the Buffer that initiated this update, necessary
			to enable remote components to filter out updates that originate d
			from their own operations
		'''
		if new_links is None:
			new_links = {}
		if links_to_remove is None:
			links_to_remove = {}
		link_update = ipaaca.converter.IULinkUpdate(iu._uid, is_delta=is_delta, revision=revision)
		link_update.new_links = new_links
		if is_delta:
			link_update.links_to_remove = links_to_remove
		link_update.writer_name = writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(link_update)
		# FIXME send the notification to the target, if the target is not the writer_name

	def _send_iu_payload_update(self, iu, is_delta, revision, new_items=None, keys_to_remove=None, writer_name="undef"):
		'''Send an IU payload update.

		Keyword arguments:
		iu -- the IU being updated
		is_delta -- whether this is an incremental update or a replacement
		revision -- the new revision number
		new_items -- a dictionary of new payload items
		keys_to_remove -- a list of the keys that shall be removed from the
		 	payload
		writer_name -- name of the Buffer that initiated this update, necessary
			to enable remote components to filter out updates that originate d
			from their own operations
		'''
		if new_items is None:
			new_items = {}
		if keys_to_remove is None:
			keys_to_remove = []
		payload_update = ipaaca.converter.IUPayloadUpdate(
			uid=iu._uid,
			revision=revision,
			is_delta=is_delta,
			payload_type=iu.payload_type)
		payload_update.new_items = new_items 
		if is_delta:
			payload_update.keys_to_remove = keys_to_remove
		payload_update.writer_name = writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(payload_update)
