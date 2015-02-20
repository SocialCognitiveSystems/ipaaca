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

import collections
import copy
import threading
import uuid

import ipaaca.converter
import ipaaca.exception
import ipaaca.misc
import ipaaca.payload


__all__ = [
	'IUAccessMode',
	'IUEventType',
	'IUPayloadType',
	'IU',
	'Message',
]

LOGGER = ipaaca.misc.get_library_logger()

IUAccessMode = ipaaca.misc.enum(
	PUSH = 'PUSH',
	REMOTE = 'REMOTE',
	MESSAGE = 'MESSAGE'
)


IUEventType = ipaaca.misc.enum(
	ADDED = 'ADDED',
	COMMITTED = 'COMMITTED',
	DELETED = 'DELETED',
	RETRACTED = 'RETRACTED',
	UPDATED = 'UPDATED',
	LINKSUPDATED = 'LINKSUPDATED',
	MESSAGE = 'MESSAGE'
)


IUPayloadType = ipaaca.misc.enum(
	JSON = 'JSON',
	STR = 'STR'
)


class IUInterface(object):

	"""Base class of all specialised IU classes."""

	def __init__(self, uid, access_mode=IUAccessMode.PUSH, read_only=False, payload_type=None):
		"""Creates an IU.

		Keyword arguments:
		uid -- unique ID of this IU
		access_mode -- access mode of this IU
		read_only -- flag indicating whether this IU is read_only or not
		"""
		self._uid = uid
		self._revision = None
		self._category = None
		self._owner_name = None
		self._committed = False
		self._retracted = False
		self._access_mode = access_mode
		self._read_only = read_only
		self._payload_type = payload_type if payload_type is not None else ipaaca.defaults.IPAACA_DEFAULT_IU_PAYLOAD_TYPE
		self._buffer = None
		# payload is not present here
		self._links = collections.defaultdict(set)

	def __str__(self):
		s = unicode(self.__class__)+"{ "
		s += "category="+("<None>" if self._category is None else self._category)+" "
		s += "uid="+self._uid+" "
		s += "(buffer="+(self.buffer.unique_name if self.buffer is not None else "<None>")+") "
		s += "owner_name=" + ("<None>" if self.owner_name is None else self.owner_name) + " "
		s += "payload={ "
		for k,v in self.payload.items():
			s += k+":'"+v+"', "
		s += "} "
		s += "links={ "
		for t,ids in self.get_all_links().items():
			s += t+":'"+str(ids)+"', "
		s += "} "
		s += "}"
		return s


	def _add_and_remove_links(self, add, remove):
		'''Just add and remove the new links in our links set, do not send an update here'''
		'''Note: Also used for remotely enforced links updates.'''
		for type in remove.keys(): self._links[type] -= set(remove[type])
		for type in add.keys(): self._links[type] |= set(add[type])
	def _replace_links(self, links):
		'''Just wipe and replace our links set, do not send an update here'''
		'''Note: Also used for remotely enforced links updates.'''
		self._links = collections.defaultdict(set)
		for type in links.keys(): self._links[type] |= set(links[type])

	def add_links(self, type, targets, writer_name=None):
		'''Attempt to add links if the conditions are met
		and send an update message. Then call the local setter.'''
		if not hasattr(targets, '__iter__'): targets=[targets]
		self._modify_links(is_delta=True, new_links={type:targets}, links_to_remove={}, writer_name=writer_name)
		self._add_and_remove_links( add={type:targets}, remove={} )
	def remove_links(self, type, targets, writer_name=None):
		'''Attempt to remove links if the conditions are met
		and send an update message. Then call the local setter.'''
		if not hasattr(targets, '__iter__'): targets=[targets]
		self._modify_links(is_delta=True, new_links={}, links_to_remove={type:targets}, writer_name=writer_name)
		self._add_and_remove_links( add={}, remove={type:targets} )
	def modify_links(self, add, remove, writer_name=None):
		'''Attempt to modify links if the conditions are met
		and send an update message. Then call the local setter.'''
		self._modify_links(is_delta=True, new_links=add, links_to_remove=remove, writer_name=writer_name)
		self._add_and_remove_links( add=add, remove=remove )
	def set_links(self, links, writer_name=None):
		'''Attempt to set (replace) links if the conditions are met
		and send an update message. Then call the local setter.'''
		self._modify_links(is_delta=False, new_links=links, links_to_remove={}, writer_name=writer_name)
		self._replace_links( links=links )
	def get_links(self, type):
		return set(self._links[type])
	def get_all_links(self):
		return copy.deepcopy(self._links)

	def _get_revision(self):
		return self._revision
	revision = property(fget=_get_revision, doc='Revision number of the IU.')

	def _get_category(self):
		return self._category
	category = property(fget=_get_category, doc='Category of the IU.')

	def _get_payload_type(self):
		return self._payload_type
	def _set_payload_type(self, type):
		if self._buffer is None:
			self._payload_type = type
		else:
			raise IpaacaException('The IU is already in a buffer, cannot change payload type anymore.')
	payload_type = property(
		fget=_get_payload_type, 
		fset=_set_payload_type,
		 doc='Type of the IU payload')

	def _get_committed(self):
		return self._committed
	committed = property(
			fget=_get_committed,
			doc='Flag indicating whether this IU has been committed to.')

	def _get_retracted(self):
		return self._retracted
	retracted = property(
			fget=_get_retracted,
			doc='Flag indicating whether this IU has been retracted.')

	def _get_uid(self):
		return self._uid
	uid = property(fget=_get_uid, doc='Unique ID of the IU.')

	def _get_access_mode(self):
		return self._access_mode
	access_mode = property(fget=_get_access_mode, doc='Access mode of the IU.')

	def _get_read_only(self):
		return self._read_only
	read_only = property(
			fget=_get_read_only,
			doc='Flag indicating whether this IU is read only.')

	def _get_buffer(self):
		return self._buffer
	def _set_buffer(self, buffer):
		if self._buffer is not None:
			raise IpaacaException('The IU is already in a buffer, cannot move it.')
		self._buffer = buffer
	buffer = property(
			fget=_get_buffer,
			fset=_set_buffer,
			doc='Buffer this IU is held in.')

	def _get_owner_name(self):
		return self._owner_name
	def _set_owner_name(self, owner_name):
		if self._owner_name is not None:
			raise Exception('The IU already has an owner name, cannot change it.')
		self._owner_name = owner_name
	owner_name = property(
			fget=_get_owner_name,
			fset=_set_owner_name,
			doc="The IU's owner's name.")


class IU(IUInterface):

	"""A local IU."""

	def __init__(self, category='undef', access_mode=IUAccessMode.PUSH, read_only=False, payload_type=None):
		super(IU, self).__init__(uid=None, access_mode=access_mode, read_only=read_only, payload_type=payload_type)
		self._revision = 1
		self.uid = str(uuid.uuid4())
		self._category = str(category)
		self.revision_lock = threading.RLock()
		self._payload = ipaaca.payload.Payload(iu=self)

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		with self.revision_lock:
			# modify links locally
			self._increase_revision_number()
			if self.is_published:
				# send update to remote holders
				self.buffer._send_iu_link_update(
						self,
						revision=self.revision,
						is_delta=is_delta,
						new_links=new_links,
						links_to_remove=links_to_remove,
						writer_name=self.owner_name if writer_name is None else writer_name)

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		"""Modify the payload: add or remove items from this payload locally and send update."""
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		with self.revision_lock:
			# set item locally
			# FIXME: Is it actually set locally?
			self._increase_revision_number()
			if self.is_published:
				#print('  _modify_payload: running send_iu_pl_upd with writer name '+str(writer_name))
				# send update to remote holders
				self.buffer._send_iu_payload_update(
						self,
						revision=self.revision,
						is_delta=is_delta,
						new_items=new_items,
						keys_to_remove=keys_to_remove,
						writer_name=self.owner_name if writer_name is None else writer_name)

	def _increase_revision_number(self):
		self._revision += 1

	def _internal_commit(self, writer_name=None):
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		with self.revision_lock:
			if not self._committed:
				self._increase_revision_number()
				self._committed = True
				if self.buffer is not None:
					self.buffer._send_iu_commission(self, writer_name=writer_name)

	def commit(self):
		"""Commit to this IU."""
		return self._internal_commit()

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl, writer_name=None):
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		with self.revision_lock:
			self._increase_revision_number()
			self._payload = ipaaca.payload.Payload(
					iu=self,
					writer_name=None if self.buffer is None else (self.buffer.unique_name if writer_name is None else writer_name),
					new_payload=new_pl)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of this IU.')

	def _get_is_published(self):
		return self.buffer is not None
	is_published = property(
			fget=_get_is_published,
			doc='Flag indicating whether this IU has been published or not.')

	def _set_buffer(self, buffer):
		if self._buffer is not None:
			raise Exception('The IU is already in a buffer, cannot move it.')
		self._buffer = buffer
		self.owner_name = buffer.unique_name
		self._payload.owner_name = buffer.unique_name
	buffer = property(
			fget=IUInterface._get_buffer,
			fset=_set_buffer,
			doc='Buffer this IU is held in.')

	def _set_uid(self, uid):
		if self._uid is not None:
			raise AttributeError('The uid of IU ' + self.uid + ' has already been set, cannot change it.')
		self._uid = uid
	uid = property(
			fget=IUInterface._get_uid,
			fset=_set_uid,
			doc='Unique ID of the IU.')


class Message(IU):
	"""Local IU of Message sub-type. Can be handled like a normal IU, but on the remote side it is only existent during the handler calls."""
	def __init__(self, category='undef', access_mode=IUAccessMode.MESSAGE, read_only=True, payload_type=None):
		super(Message, self).__init__(category=str(category), access_mode=access_mode, read_only=read_only, payload_type=payload_type)

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		if self.is_published:
			LOGGER.info('Info: modifying a Message after sending has no global effects')

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		if self.is_published:
			LOGGER.info('Info: modifying a Message after sending has no global effects')

	def _increase_revision_number(self):
		self._revision += 1

	def _internal_commit(self, writer_name=None):
		if self.is_published:
			LOGGER.info('Info: committing to a Message after sending has no global effects')

	def commit(self):
		return self._internal_commit()

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl, writer_name=None):
		if self.is_published:
			LOGGER.info('Info: modifying a Message after sending has no global effects')
		else:
			if self.committed:
				raise ipaaca.exception.IUCommittedError(self)
			with self.revision_lock:
				self._increase_revision_number()
				self._payload = ipaaca.payload.Payload(
						iu=self,
						writer_name=None if self.buffer is None else (self.buffer.unique_name if writer_name is None else writer_name),
						new_payload=new_pl)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of this IU.')

	def _get_is_published(self):
		return self.buffer is not None
	is_published = property(
			fget=_get_is_published,
			doc='Flag indicating whether this IU has been published or not.')

	def _set_buffer(self, buffer):
		if self._buffer is not None:
			raise Exception('The IU is already in a buffer, cannot move it.')
		self._buffer = buffer
		self.owner_name = buffer.unique_name
		self._payload.owner_name = buffer.unique_name
	buffer = property(
			fget=IUInterface._get_buffer,
			fset=_set_buffer,
			doc='Buffer this IU is held in.')

	def _set_uid(self, uid):
		if self._uid is not None:
			raise AttributeError('The uid of IU ' + self.uid + ' has already been set, cannot change it.')
		self._uid = uid
	uid = property(
			fget=IUInterface._get_uid,
			fset=_set_uid,
			doc='Unique ID of the IU.')


class RemoteMessage(IUInterface):

	"""A remote IU with access mode 'MESSAGE'."""

	def __init__(self, uid, revision, read_only, owner_name, category, payload_type, committed, payload, links):
		super(RemoteMessage, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only, payload_type=payload_type)
		self._revision = revision
		self._category = category
		self.owner_name = owner_name
		self._committed = committed
		self._retracted = False
		# NOTE Since the payload is an already-existant Payload which we didn't modify ourselves,
		#  don't try to invoke any modification checks or network updates ourselves either.
		#  We are just receiving it here and applying the new data.
		self._payload = ipaaca.payload.Payload(iu=self, new_payload=payload, omit_init_update_message=True)
		self._links = links

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		LOGGER.info('Info: modifying a RemoteMessage only has local effects')

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		LOGGER.info('Info: modifying a RemoteMessage only has local effects')

	def commit(self):
		LOGGER.info('Info: committing to a RemoteMessage only has local effects')

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl):
		LOGGER.info('Info: modifying a RemoteMessage only has local effects')
		self._payload = ipaaca.payload.Payload(iu=self, new_payload=new_pl, omit_init_update_message=True)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of the IU.')

	def _apply_link_update(self, update):
		"""Apply a IULinkUpdate to the IU."""
		LOGGER.warning('Warning: should never be called: RemoteMessage._apply_link_update')
		self._revision = update.revision
		if update.is_delta:
			self._add_and_remove_links(add=update.new_links, remove=update.links_to_remove)
		else:
			self._replace_links(links=update.new_links)

	def _apply_update(self, update):
		"""Apply a IUPayloadUpdate to the IU."""
		LOGGER.warning('Warning: should never be called: RemoteMessage._apply_update')
		self._revision = update.revision
		if update.is_delta:
			for k in update.keys_to_remove: self.payload._remotely_enforced_delitem(k)
			for k, v in update.new_items.items(): self.payload._remotely_enforced_setitem(k, v)
		else:
			# NOTE Please read the comment in the constructor
			self._payload = ipaaca.payload.Payload(iu=self, new_payload=update.new_items, omit_init_update_message=True)

	def _apply_commission(self):
		"""Apply commission to the IU"""
		LOGGER.warning('Warning: should never be called: RemoteMessage._apply_commission')
		self._committed = True

	def _apply_retraction(self):
		"""Apply retraction to the IU"""
		LOGGER.warning('Warning: should never be called: RemoteMessage._apply_retraction')
		self._retracted = True


class RemotePushIU(IUInterface):

	"""A remote IU with access mode 'PUSH'."""

	def __init__(self, uid, revision, read_only, owner_name, category, payload_type, committed, payload, links):
		super(RemotePushIU, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only, payload_type=payload_type)
		self._revision = revision
		self._category = category
		self.owner_name = owner_name
		self._committed = committed
		self._retracted = False
		# NOTE Since the payload is an already-existant Payload which we didn't modify ourselves,
		#  don't try to invoke any modification checks or network updates ourselves either.
		#  We are just receiving it here and applying the new data.
		self._payload = ipaaca.payload.Payload(iu=self, new_payload=payload, omit_init_update_message=True)
		self._links = links

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		"""Modify the links: add or remove item from this payload remotely and send update."""
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		if self.read_only:
			raise ipaaca.exception.IUReadOnlyError(self)
		requested_update = ipaaca.converter.IULinkUpdate(
				uid=self.uid,
				revision=self.revision,
				is_delta=is_delta,
				writer_name=self.buffer.unique_name,
				new_links=new_links,
				links_to_remove=links_to_remove)
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updateLinks(requested_update)
		if new_revision == 0:
			raise ipaaca.exception.IUUpdateFailedError(self)
		else:
			self._revision = new_revision

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		"""Modify the payload: add or remove item from this payload remotely and send update."""
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		if self.read_only:
			raise ipaaca.exception.IUReadOnlyError(self)
		requested_update = ipaaca.converter.IUPayloadUpdate(
				uid=self.uid,
				revision=self.revision,
				payload_type=self.payload_type,
				is_delta=is_delta,
				writer_name=self.buffer.unique_name,
				new_items=new_items,
				keys_to_remove=keys_to_remove)
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise ipaaca.exception.IUUpdateFailedError(self)
		else:
			self._revision = new_revision

	def commit(self):
		"""Commit to this IU."""
		if self.read_only:
			raise ipaaca.exception.IUReadOnlyError(self)
		if self._committed:
			# ignore commit requests when already committed
			return
		else:
			commission_request = ipaaca_pb2.IUCommission()
			commission_request.uid = self.uid
			commission_request.revision = self.revision
			commission_request.writer_name = self.buffer.unique_name
			remote_server = self.buffer._get_remote_server(self)
			new_revision = remote_server.commit(commission_request)
			if new_revision == 0:
				raise ipaaca.exception.IUUpdateFailedError(self)
			else:
				self._revision = new_revision
				self._committed = True

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl):
		if self.committed:
			raise ipaaca.exception.IUCommittedError(self)
		if self.read_only:
			raise ipaaca.exception.IUReadOnlyError(self)
		requested_update = ipaaca.converter.IUPayloadUpdate(
				uid=self.uid,
				revision=self.revision,
				payload_type=self.payload_type,
				is_delta=False,
				writer_name=self.buffer.unique_name,
				new_items=new_pl,
				keys_to_remove=[])
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise ipaaca.exception.IUUpdateFailedError(self)
		else:
			self._revision = new_revision
			# NOTE Please read the comment in the constructor
			self._payload = ipaaca.payload.Payload(iu=self, new_payload=new_pl, omit_init_update_message=True)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of the IU.')

	def _apply_link_update(self, update):
		"""Apply a IULinkUpdate to the IU."""
		self._revision = update.revision
		if update.is_delta:
			self._add_and_remove_links(add=update.new_links, remove=update.links_to_remove)
		else:
			self._replace_links(links=update.new_links)

	def _apply_update(self, update):
		"""Apply a IUPayloadUpdate to the IU."""
		self._revision = update.revision
		if update.is_delta:
			for k in update.keys_to_remove: self.payload._remotely_enforced_delitem(k)
			for k, v in update.new_items.items(): self.payload._remotely_enforced_setitem(k, v)
		else:
			# NOTE Please read the comment in the constructor
			self._payload = ipaaca.payload.Payload(iu=self, new_payload=update.new_items, omit_init_update_message=True)

	def _apply_commission(self):
		"""Apply commission to the IU"""
		self._committed = True

	def _apply_retraction(self):
		"""Apply retraction to the IU"""
		self._retracted = True
