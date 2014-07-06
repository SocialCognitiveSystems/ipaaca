# -*- coding: utf-8 -*-

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".
#
# Copyright (c) 2009-2013 Sociable Agents Group
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

from __future__ import print_function, division

import logging
import sys
import threading
import uuid
import collections
import copy
import time

import rsb
import rsb.converter

import ipaaca_pb2

_DEFAULT_PAYLOAD_UPDATE_TIMEOUT = 0.1

# IDEAS
#  We should think about relaying the update event (or at least the
#  affected keys in the payload / links) to the event handlers!

# THOUGHTS
#  Output buffers could generate UIDs for IUs on request, without
#  publishing them at that time. Then UID could then be used
#  for internal links etc. The IU may be published later through
#  the same buffer that allocated the UID.

# WARNINGS
#  category is now the FIRST argument for IU constructors

__all__ = [
	'IUEventType',
	'IUAccessMode',
	'InputBuffer', 'OutputBuffer',
	'IU',
	'IUPublishedError', 'IUUpdateFailedError', 'IUCommittedError', 'IUReadOnlyError', 'IUNotFoundError', 'IUResendFailedError',
	'logger'
]


## --- Utilities -------------------------------------------------------------


def enum(*sequential, **named):
	"""Create an enum type.

	Based on suggestion of Alec Thomas on stackoverflow.com:
	http://stackoverflow.com/questions/36932/
		whats-the-best-way-to-implement-an-enum-in-python/1695250#1695250
	"""
	enums = dict(zip(sequential, range(len(sequential))), **named)
	return type('Enum', (), enums)


def pack_typed_payload_item(protobuf_object, key, value):
	protobuf_object.key = key
	protobuf_object.value = value
	protobuf_object.type = 'str' # TODO: more types


def unpack_typed_payload_item(protobuf_object):
	# TODO: more types
	return (protobuf_object.key, protobuf_object.value)


class IpaacaLoggingHandler(logging.Handler):

	def __init__(self, level=logging.DEBUG):
		logging.Handler.__init__(self, level)

	def emit(self, record):
		meta = '[ipaaca] (' + str(record.levelname) + ') '
		msg = str(record.msg.format(record.args))
		print(meta + msg)


## --- Global Definitions ----------------------------------------------------


IUEventType = enum(
	ADDED = 'ADDED',
	COMMITTED = 'COMMITTED',
	DELETED = 'DELETED',
	RETRACTED = 'RETRACTED',
	UPDATED = 'UPDATED',
	LINKSUPDATED = 'LINKSUPDATED',
	MESSAGE = 'MESSAGE'
)


IUAccessMode = enum(
	"PUSH",
	"REMOTE",
	"MESSAGE"
)


## --- Errors and Exceptions -------------------------------------------------


class IUPublishedError(Exception):
	"""Error publishing of an IU failed since it is already in the buffer."""
	def __init__(self, iu):
		super(IUPublishedError, self).__init__('IU ' + str(iu.uid) + ' is already present in the output buffer.')


class IUUpdateFailedError(Exception):
	"""Error indicating that a remote IU update failed."""
	def __init__(self, iu):
		super(IUUpdateFailedError, self).__init__('Remote update failed for IU ' + str(iu.uid) + '.')

class IUResendFailedError(Exception):
	"""Error indicating that a remote IU resend failed."""
	def __init__(self, iu):
		super(IUResendFailedError, self).__init__('Remote resend failed for IU ' + str(iu.uid) + '.')

class IUCommittedError(Exception):
	"""Error indicating that an IU is immutable because it has been committed to."""
	def __init__(self, iu):
		super(IUCommittedError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it has been committed to.')


class IUReadOnlyError(Exception):
	"""Error indicating that an IU is immutable because it is 'read only'."""
	def __init__(self, iu):
		super(IUReadOnlyError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it is read-only.')

class IUNotFoundError(Exception):
	"""Error indicating that an IU UID was unexpectedly not found in an internal store."""
	def __init__(self, iu_uid):
		super(IUNotFoundError, self).__init__('Lookup of IU ' + str(iu_uid) + ' failed.')

class IUPayloadLockTimeoutError(Exception):
	"""Error indicating that exclusive access to the Payload could not be obtained in time."""
	def __init__(self, iu):
		super(IUPayloadLockTimeoutError, self).__init__('Timeout while accessing payload of IU ' + str(iu.uid) + '.')

class IUPayloadLockedError(Exception):
	"""Error indicating that exclusive access to the Payload could not be obtained because someone actually locked it."""
	def __init__(self, iu):
		super(IUPayloadLockedError, self).__init__('IU '+str(iu.uid)+' was locked during access attempt.')


## --- Generation Architecture -----------------------------------------------

class Payload(dict):
	def __init__(self, iu, writer_name=None, new_payload=None, omit_init_update_message=False, update_timeout=_DEFAULT_PAYLOAD_UPDATE_TIMEOUT):
		self.iu = iu
		pl1 = {} if new_payload is None else new_payload
		pl = {}
		for k,v in pl1.items():
			if type(k)==str:
				k=unicode(k,'utf8')
			if type(v)==str:
				v=unicode(v,'utf8')
			pl[k] = v
		# NOTE omit_init_update_message is necessary to prevent checking for
		#   exceptions and sending updates in the case where we just receive
		#   a whole new payload from the remote side and overwrite it locally.
		for k, v in pl.items():
			dict.__setitem__(self, k, v)
		if (not omit_init_update_message) and (self.iu.buffer is not None):
			self.iu._modify_payload(is_delta=False, new_items=pl, keys_to_remove=[], writer_name=writer_name)
		self._update_on_every_change = True
		self._collected_modifications = {}
		self._collected_removals = []
		self._update_timeout = update_timeout
		self._batch_update_writer_name = None  # name of remote buffer or None
		self._batch_update_lock = threading.RLock()
		self._batch_update_cond = threading.Condition(threading.RLock())

	def merge(self, payload, writer_name=None):
		self._batch_update_lock.acquire(True)
		#if not self._batch_update_lock.acquire(False):
		#	print('Someone failed a lock trying to merge '+str(payload.keys()))
		#	raise IUPayloadLockedError(self.iu)
		#print("Payload.merge() IN, Merging "+str(payload.keys()))
		for k, v in payload.items():
			if type(k)==str:
				k=unicode(k,'utf8')
			if type(v)==str:
				v=unicode(v,'utf8')
		self.iu._modify_payload(is_delta=True, new_items=payload, keys_to_remove=[], writer_name=writer_name)
		r = dict.update(self, payload) # batch update
		#print("Payload.merge() OUT")
		self._batch_update_lock.release()
		return r

	def __setitem__(self, k, v, writer_name=None):
		self._batch_update_lock.acquire(True)
		#if not self._batch_update_lock.acquire(False):
		#	print('Someone failed a lock trying to set '+k+' to '+v)
		#	raise IUPayloadLockedError(self.iu)
		#print("Payload.__setitem__() IN, Setting "+k+' to '+v)
		#print("  by writer "+str(writer_name))
		if type(k)==str:
			k=unicode(k,'utf8')
		if type(v)==str:
			v=unicode(v,'utf8')
		if self._update_on_every_change:
			#print("  running _modify_payload with writer name "+str(writer_name))
			self.iu._modify_payload(is_delta=True, new_items={k:v}, keys_to_remove=[], writer_name=writer_name)
		else: # Collect additions/modifications
			self._batch_update_writer_name = writer_name
			self._collected_modifications[k] = v
		r = dict.__setitem__(self, k, v)
		#print("Payload.__setitem__() OUT")
		self._batch_update_lock.release()
		return r

	def __delitem__(self, k, writer_name=None):
		self._batch_update_lock.acquire(True)
		#if not self._batch_update_lock.acquire(False):
		#	print('Someone failed a lock trying to del '+k)
		#	raise IUPayloadLockedError(self.iu)
		#print("Payload.__delitem__() IN, Deleting "+k)
		if type(k)==str:
			k=unicode(k,'utf8')
		if self._update_on_every_change:
			self.iu._modify_payload(is_delta=True, new_items={}, keys_to_remove=[k], writer_name=writer_name)
		else: # Collect additions/modifications
			self._batch_update_writer_name = writer_name
			self._collected_removals.append(k)
		r = dict.__delitem__(self, k)
		#print("Payload.__delitem__() OUT")
		self._batch_update_lock.release()
		return r

	# Context-manager based batch updates, not yet thread-safe (on remote updates)
	def __enter__(self):
		#print('running Payload.__enter__()')
		self._wait_batch_update_lock(self._update_timeout)
		self._update_on_every_change = False

	def __exit__(self, type, value, traceback):
		#print('running Payload.__exit__()')
		self.iu._modify_payload(is_delta=True, new_items=self._collected_modifications, keys_to_remove=self._collected_removals, writer_name=self._batch_update_writer_name)
		self._collected_modifications = {}
		self._collected_removals = []
		self._update_on_every_change = True
		self._batch_update_writer_name = None
		self._batch_update_lock.release()

	def _remotely_enforced_setitem(self, k, v):
		"""Sets an item when requested remotely."""
		return dict.__setitem__(self, k, v)

	def _remotely_enforced_delitem(self, k):
		"""Deletes an item when requested remotely."""
		return dict.__delitem__(self, k)

	def _wait_batch_update_lock(self, timeout):
		# wait lock with time-out http://stackoverflow.com/a/8393033
		with self._batch_update_cond:
			current_time = start_time = time.time()
			while current_time < start_time + timeout:
				if self._batch_update_lock.acquire(False):
					return True
				else:
					self._batch_update_cond.wait(timeout - current_time + start_time)
					current_time = time.time()
		raise IUPayloadLockTimeoutError(self.iu)


class IUInterface(object): #{{{

	"""Base class of all specialised IU classes."""

	def __init__(self, uid, access_mode=IUAccessMode.PUSH, read_only=False):
		"""Creates an IU.

		Keyword arguments:
		uid -- unique ID of this IU
		access_mode -- access mode of this IU
		read_only -- flag indicating whether this IU is read_only or not
		"""
		self._uid = uid
		self._revision = None
		self._category = None
		self._payload_type = None
		self._owner_name = None
		self._committed = False
		self._retracted = False
		self._access_mode = access_mode
		self._read_only = read_only
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
	payload_type = property(fget=_get_payload_type, doc='Type of the IU payload')

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
			raise Exception('The IU is already in a buffer, cannot move it.')
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
#}}}

class IU(IUInterface):#{{{

	"""A local IU."""

	def __init__(self, category='undef', access_mode=IUAccessMode.PUSH, read_only=False, _payload_type='MAP'):
		super(IU, self).__init__(uid=None, access_mode=access_mode, read_only=read_only)
		self._revision = 1
		self.uid = str(uuid.uuid4())
		self._category = category
		self._payload_type = _payload_type
		self.revision_lock = threading.RLock()
		self._payload = Payload(iu=self)

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		if self.committed:
			raise IUCommittedError(self)
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
			raise IUCommittedError(self)
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
			raise IUCommittedError(self)
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
			raise IUCommittedError(self)
		with self.revision_lock:
			self._increase_revision_number()
			self._payload = Payload(
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

#}}}

class Message(IU):#{{{
	"""Local IU of Message sub-type. Can be handled like a normal IU, but on the remote side it is only existent during the handler calls."""
	def __init__(self, category='undef', access_mode=IUAccessMode.MESSAGE, read_only=True, _payload_type='MAP'):
		super(Message, self).__init__(category=category, access_mode=access_mode, read_only=read_only, _payload_type=_payload_type)

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		if self.is_published:
			logger.info('Info: modifying a Message after sending has no global effects')

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		if self.is_published:
			logger.info('Info: modifying a Message after sending has no global effects')

	def _increase_revision_number(self):
		self._revision += 1

	def _internal_commit(self, writer_name=None):
		if self.is_published:
			logger.info('Info: committing to a Message after sending has no global effects')

	def commit(self):
		return self._internal_commit()

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl, writer_name=None):
		if self.is_published:
			logger.info('Info: modifying a Message after sending has no global effects')
		else:
			if self.committed:
				raise IUCommittedError(self)
			with self.revision_lock:
				self._increase_revision_number()
				self._payload = Payload(
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
#}}}

class RemoteMessage(IUInterface):#{{{

	"""A remote IU with access mode 'MESSAGE'."""

	def __init__(self, uid, revision, read_only, owner_name, category, payload_type, committed, payload, links):
		super(RemoteMessage, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only)
		self._revision = revision
		self._category = category
		self.owner_name = owner_name
		self._payload_type = payload_type
		self._committed = committed
		self._retracted = False
		# NOTE Since the payload is an already-existant Payload which we didn't modify ourselves,
		#  don't try to invoke any modification checks or network updates ourselves either.
		#  We are just receiving it here and applying the new data.
		self._payload = Payload(iu=self, new_payload=payload, omit_init_update_message=True)
		self._links = links

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		logger.info('Info: modifying a RemoteMessage only has local effects')

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		logger.info('Info: modifying a RemoteMessage only has local effects')

	def commit(self):
		logger.info('Info: committing to a RemoteMessage only has local effects')

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl):
		logger.info('Info: modifying a RemoteMessage only has local effects')
		self._payload = Payload(iu=self, new_payload=new_pl, omit_init_update_message=True)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of the IU.')

	def _apply_link_update(self, update):
		"""Apply a IULinkUpdate to the IU."""
		logger.warning('Warning: should never be called: RemoteMessage._apply_link_update')
		self._revision = update.revision
		if update.is_delta:
			self._add_and_remove_links(add=update.new_links, remove=update.links_to_remove)
		else:
			self._replace_links(links=update.new_links)

	def _apply_update(self, update):
		"""Apply a IUPayloadUpdate to the IU."""
		logger.warning('Warning: should never be called: RemoteMessage._apply_update')
		self._revision = update.revision
		if update.is_delta:
			for k in update.keys_to_remove: self.payload._remotely_enforced_delitem(k)
			for k, v in update.new_items.items(): self.payload._remotely_enforced_setitem(k, v)
		else:
			# NOTE Please read the comment in the constructor
			self._payload = Payload(iu=self, new_payload=update.new_items, omit_init_update_message=True)

	def _apply_commission(self):
		"""Apply commission to the IU"""
		logger.warning('Warning: should never be called: RemoteMessage._apply_commission')
		self._committed = True

	def _apply_retraction(self):
		"""Apply retraction to the IU"""
		logger.warning('Warning: should never be called: RemoteMessage._apply_retraction')
		self._retracted = True
#}}}

class RemotePushIU(IUInterface):#{{{

	"""A remote IU with access mode 'PUSH'."""

	def __init__(self, uid, revision, read_only, owner_name, category, payload_type, committed, payload, links):
		super(RemotePushIU, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only)
		self._revision = revision
		self._category = category
		self.owner_name = owner_name
		self._payload_type = payload_type
		self._committed = committed
		self._retracted = False
		# NOTE Since the payload is an already-existant Payload which we didn't modify ourselves,
		#  don't try to invoke any modification checks or network updates ourselves either.
		#  We are just receiving it here and applying the new data.
		self._payload = Payload(iu=self, new_payload=payload, omit_init_update_message=True)
		self._links = links

	def _modify_links(self, is_delta=False, new_links={}, links_to_remove={}, writer_name=None):
		"""Modify the links: add or remove item from this payload remotely and send update."""
		if self.committed:
			raise IUCommittedError(self)
		if self.read_only:
			raise IUReadOnlyError(self)
		requested_update = IULinkUpdate(
				uid=self.uid,
				revision=self.revision,
				is_delta=is_delta,
				writer_name=self.buffer.unique_name,
				new_links=new_links,
				links_to_remove=links_to_remove)
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updateLinks(requested_update)
		if new_revision == 0:
			raise IUUpdateFailedError(self)
		else:
			self._revision = new_revision

	def _modify_payload(self, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
		"""Modify the payload: add or remove item from this payload remotely and send update."""
		if self.committed:
			raise IUCommittedError(self)
		if self.read_only:
			raise IUReadOnlyError(self)
		requested_update = IUPayloadUpdate(
				uid=self.uid,
				revision=self.revision,
				is_delta=is_delta,
				writer_name=self.buffer.unique_name,
				new_items=new_items,
				keys_to_remove=keys_to_remove)
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise IUUpdateFailedError(self)
		else:
			self._revision = new_revision

	def commit(self):
		"""Commit to this IU."""
		if self.read_only:
			raise IUReadOnlyError(self)
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
				raise IUUpdateFailedError(self)
			else:
				self._revision = new_revision
				self._committed = True

	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl):
		if self.committed:
			raise IUCommittedError(self)
		if self.read_only:
			raise IUReadOnlyError(self)
		requested_update = IUPayloadUpdate(
				uid=self.uid,
				revision=self.revision,
				is_delta=False,
				writer_name=self.buffer.unique_name,
				new_items=new_pl,
				keys_to_remove=[])
		remote_server = self.buffer._get_remote_server(self)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise IUUpdateFailedError(self)
		else:
			self._revision = new_revision
			# NOTE Please read the comment in the constructor
			self._payload = Payload(iu=self, new_payload=new_pl, omit_init_update_message=True)
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
			self._payload = Payload(iu=self, new_payload=update.new_items, omit_init_update_message=True)

	def _apply_commission(self):
		"""Apply commission to the IU"""
		self._committed = True

	def _apply_retraction(self):
		"""Apply retraction to the IU"""
		self._retracted = True
#}}}


class IntConverter(rsb.converter.Converter):#{{{
	"""Convert Python int objects to Protobuf ints and vice versa."""
	def __init__(self, wireSchema="int", dataType=int):
		super(IntConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, value):
		pbo = ipaaca_pb2.IntMessage()
		pbo.value = value
		return bytearray(pbo.SerializeToString()), self.wireSchema

	def deserialize(self, byte_stream, ws):
		pbo = ipaaca_pb2.IntMessage()
		pbo.ParseFromString( str(byte_stream) )
		return pbo.value
#}}}


class IUConverter(rsb.converter.Converter):#{{{
	'''
	Converter class for Full IU representations
	wire:bytearray <-> wire-schema:ipaaca-full-iu <-> class ipaacaRSB.IU
	'''
	def __init__(self, wireSchema="ipaaca-iu", dataType=IU):
		super(IUConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, iu):
		pbo = ipaaca_pb2.IU()
		pbo.uid = iu._uid
		pbo.revision = iu._revision
		pbo.category = iu._category
		pbo.payload_type = iu._payload_type
		pbo.owner_name = iu._owner_name
		pbo.committed = iu._committed
		am=ipaaca_pb2.IU.PUSH #default
		if iu._access_mode == IUAccessMode.MESSAGE:
			am=ipaaca_pb2.IU.MESSAGE
		# TODO add other types later
		pbo.access_mode = am
		pbo.read_only = iu._read_only
		for k,v in iu._payload.items():
			entry = pbo.payload.add()
			pack_typed_payload_item(entry, k, v)
		for type_ in iu._links.keys():
			linkset = pbo.links.add()
			linkset.type = type_
			linkset.targets.extend(iu._links[type_])
		ws = "ipaaca-messageiu" if iu._access_mode == IUAccessMode.MESSAGE else self.wireSchema
		return bytearray(pbo.SerializeToString()), ws

	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		#print('IUConverter.deserialize got a '+str(type)+' over wireSchema '+ws)
		if type == IU or type == Message:
			pbo = ipaaca_pb2.IU()
			pbo.ParseFromString( str(byte_stream) )
			if pbo.access_mode ==  ipaaca_pb2.IU.PUSH:
				_payload = {}
				for entry in pbo.payload:
					k, v = unpack_typed_payload_item(entry)
					_payload[k] = v
				_links = collections.defaultdict(set)
				for linkset in pbo.links:
					for target_uid in linkset.targets:
						_links[linkset.type].add(target_uid)
				remote_push_iu = RemotePushIU(
						uid=pbo.uid,
						revision=pbo.revision,
						read_only = pbo.read_only,
						owner_name = pbo.owner_name,
						category = pbo.category,
						payload_type = pbo.payload_type,
						committed = pbo.committed,
						payload=_payload,
						links=_links
					)
				return remote_push_iu
			elif pbo.access_mode ==  ipaaca_pb2.IU.MESSAGE:
				_payload = {}
				for entry in pbo.payload:
					k, v = unpack_typed_payload_item(entry)
					_payload[k] = v
				_links = collections.defaultdict(set)
				for linkset in pbo.links:
					for target_uid in linkset.targets:
						_links[linkset.type].add(target_uid)
				remote_message = RemoteMessage(
						uid=pbo.uid,
						revision=pbo.revision,
						read_only = pbo.read_only,
						owner_name = pbo.owner_name,
						category = pbo.category,
						payload_type = pbo.payload_type,
						committed = pbo.committed,
						payload=_payload,
						links=_links
					)
				return remote_message
			else:
				raise Exception("We can only handle IUs with access mode 'PUSH' or 'MESSAGE' for now!")
		else:
			raise ValueError("Inacceptable dataType %s" % type)
#}}}

class MessageConverter(rsb.converter.Converter):#{{{
	'''
	Converter class for Full IU representations
	wire:bytearray <-> wire-schema:ipaaca-full-iu <-> class ipaacaRSB.IU
	'''
	def __init__(self, wireSchema="ipaaca-messageiu", dataType=Message):
		super(IUConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, iu):
		pbo = ipaaca_pb2.IU()
		pbo.uid = iu._uid
		pbo.revision = iu._revision
		pbo.category = iu._category
		pbo.payload_type = iu._payload_type
		pbo.owner_name = iu._owner_name
		pbo.committed = iu._committed
		am=ipaaca_pb2.IU.PUSH #default
		if iu._access_mode == IUAccessMode.MESSAGE:
			am=ipaaca_pb2.IU.MESSAGE
		# TODO add other types later
		pbo.access_mode = am
		pbo.read_only = iu._read_only
		for k,v in iu._payload.items():
			entry = pbo.payload.add()
			pack_typed_payload_item(entry, k, v)
		for type_ in iu._links.keys():
			linkset = pbo.links.add()
			linkset.type = type_
			linkset.targets.extend(iu._links[type_])
		ws = "ipaaca-messageiu" if iu._access_mode == IUAccessMode.MESSAGE else self.wireSchema
		return bytearray(pbo.SerializeToString()), ws

	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		#print('MessageConverter.deserialize got a '+str(type)+' over wireSchema '+ws)
		if type == IU or type == Message:
			pbo = ipaaca_pb2.IU()
			pbo.ParseFromString( str(byte_stream) )
			if pbo.access_mode ==  ipaaca_pb2.IU.PUSH:
				_payload = {}
				for entry in pbo.payload:
					k, v = unpack_typed_payload_item(entry)
					_payload[k] = v
				_links = collections.defaultdict(set)
				for linkset in pbo.links:
					for target_uid in linkset.targets:
						_links[linkset.type].add(target_uid)
				remote_push_iu = RemotePushIU(
						uid=pbo.uid,
						revision=pbo.revision,
						read_only = pbo.read_only,
						owner_name = pbo.owner_name,
						category = pbo.category,
						payload_type = pbo.payload_type,
						committed = pbo.committed,
						payload=_payload,
						links=_links
					)
				return remote_push_iu
			elif pbo.access_mode ==  ipaaca_pb2.IU.MESSAGE:
				_payload = {}
				for entry in pbo.payload:
					k, v = unpack_typed_payload_item(entry)
					_payload[k] = v
				_links = collections.defaultdict(set)
				for linkset in pbo.links:
					for target_uid in linkset.targets:
						_links[linkset.type].add(target_uid)
				remote_message = RemoteMessage(
						uid=pbo.uid,
						revision=pbo.revision,
						read_only = pbo.read_only,
						owner_name = pbo.owner_name,
						category = pbo.category,
						payload_type = pbo.payload_type,
						committed = pbo.committed,
						payload=_payload,
						links=_links
					)
				return remote_message
			else:
				raise Exception("We can only handle IUs with access mode 'PUSH' or 'MESSAGE' for now!")
		else:
			raise ValueError("Inacceptable dataType %s" % type)
#}}}


class IULinkUpdate(object):#{{{

	def __init__(self, uid, revision, is_delta, writer_name="undef", new_links=None, links_to_remove=None):
		super(IULinkUpdate, self).__init__()
		self.uid = uid
		self.revision = revision
		self.writer_name = writer_name
		self.is_delta = is_delta
		self.new_links = collections.defaultdict(set) if new_links is None else collections.defaultdict(set, new_links)
		self.links_to_remove = collections.defaultdict(set) if links_to_remove is None else collections.defaultdict(set, links_to_remove)

	def __str__(self):
		s =  'LinkUpdate(' + 'uid=' + self.uid + ', '
		s += 'revision='+str(self.revision)+', '
		s += 'writer_name='+str(self.writer_name)+', '
		s += 'is_delta='+str(self.is_delta)+', '
		s += 'new_links = '+str(self.new_links)+', '
		s += 'links_to_remove = '+str(self.links_to_remove)+')'
		return s
#}}}

class IUPayloadUpdate(object):#{{{

	def __init__(self, uid, revision, is_delta, writer_name="undef", new_items=None, keys_to_remove=None):
		super(IUPayloadUpdate, self).__init__()
		self.uid = uid
		self.revision = revision
		self.writer_name = writer_name
		self.is_delta = is_delta
		self.new_items = {} if new_items is None else new_items
		self.keys_to_remove = [] if keys_to_remove is None else keys_to_remove

	def __str__(self):
		s =  'PayloadUpdate(' + 'uid=' + self.uid + ', '
		s += 'revision='+str(self.revision)+', '
		s += 'writer_name='+str(self.writer_name)+', '
		s += 'is_delta='+str(self.is_delta)+', '
		s += 'new_items = '+str(self.new_items)+', '
		s += 'keys_to_remove = '+str(self.keys_to_remove)+')'
		return s
#}}}

class IULinkUpdateConverter(rsb.converter.Converter):#{{{
	def __init__(self, wireSchema="ipaaca-iu-link-update", dataType=IULinkUpdate):
		super(IULinkUpdateConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, iu_link_update):
		pbo = ipaaca_pb2.IULinkUpdate()
		pbo.uid = iu_link_update.uid
		pbo.writer_name = iu_link_update.writer_name
		pbo.revision = iu_link_update.revision
		for type_ in iu_link_update.new_links.keys():
			linkset = pbo.new_links.add()
			linkset.type = type_
			linkset.targets.extend(iu_link_update.new_links[type_])
		for type_ in iu_link_update.links_to_remove.keys():
			linkset = pbo.links_to_remove.add()
			linkset.type = type_
			linkset.targets.extend(iu_link_update.links_to_remove[type_])
		pbo.is_delta = iu_link_update.is_delta
		return bytearray(pbo.SerializeToString()), self.wireSchema

	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		if type == IULinkUpdate:
			pbo = ipaaca_pb2.IULinkUpdate()
			pbo.ParseFromString( str(byte_stream) )
			logger.debug('received an IULinkUpdate for revision '+str(pbo.revision))
			iu_link_up = IULinkUpdate( uid=pbo.uid, revision=pbo.revision, writer_name=pbo.writer_name, is_delta=pbo.is_delta)
			for entry in pbo.new_links:
				iu_link_up.new_links[str(entry.type)] = set(entry.targets)
			for entry in pbo.links_to_remove:
				iu_link_up.links_to_remove[str(entry.type)] = set(entry.targets)
			return iu_link_up
		else:
			raise ValueError("Inacceptable dataType %s" % type)
#}}}

class IUPayloadUpdateConverter(rsb.converter.Converter):#{{{
	def __init__(self, wireSchema="ipaaca-iu-payload-update", dataType=IUPayloadUpdate):
		super(IUPayloadUpdateConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, iu_payload_update):
		pbo = ipaaca_pb2.IUPayloadUpdate()
		pbo.uid = iu_payload_update.uid
		pbo.writer_name = iu_payload_update.writer_name
		pbo.revision = iu_payload_update.revision
		for k,v in iu_payload_update.new_items.items():
			entry = pbo.new_items.add()
			pack_typed_payload_item(entry, k, v)
		pbo.keys_to_remove.extend(iu_payload_update.keys_to_remove)
		pbo.is_delta = iu_payload_update.is_delta
		return bytearray(pbo.SerializeToString()), self.wireSchema

	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		if type == IUPayloadUpdate:
			pbo = ipaaca_pb2.IUPayloadUpdate()
			pbo.ParseFromString( str(byte_stream) )
			logger.debug('received an IUPayloadUpdate for revision '+str(pbo.revision))
			iu_up = IUPayloadUpdate( uid=pbo.uid, revision=pbo.revision, writer_name=pbo.writer_name, is_delta=pbo.is_delta)
			for entry in pbo.new_items:
				k, v = unpack_typed_payload_item(entry)
				iu_up.new_items[k] = v
			iu_up.keys_to_remove = pbo.keys_to_remove[:]
			return iu_up
		else:
			raise ValueError("Inacceptable dataType %s" % type)
#}}}


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

	def __init__(self, owning_component_name, participant_config=None):
		'''Create a Buffer.

		Keyword arguments:
		owning_compontent_name -- name of the entity that owns this Buffer
		participant_config -- RSB configuration
		'''
		super(Buffer, self).__init__()
		self._owning_component_name = owning_component_name
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
		handler = IUEventHandler(handler_function=handler_function, for_event_types=for_event_types, for_categories=for_categories)
		self._iu_event_handlers.append(handler)

	def call_iu_event_handlers(self, uid, local, event_type, category):
		"""Call registered IU event handler functions registered for this event_type and category."""
		for h in self._iu_event_handlers:
			h.call(self, uid, local=local, event_type=event_type, category=category)

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

	def __init__(self, owning_component_name, category_interests=None, participant_config=None):
		'''Create an InputBuffer.

		Keyword arguments:
		owning_compontent_name -- name of the entity that owns this InputBuffer
		category_interests -- list of IU categories this Buffer is interested in
		participant_config = RSB configuration
		'''
		super(InputBuffer, self).__init__(owning_component_name, participant_config)
		self._unique_name = '/ipaaca/component/'+str(owning_component_name)+'ID'+self._uuid+'/IB'
		self._listener_store = {} # one per IU category
		self._remote_server_store = {} # one per remote-IU-owning Component
		self._category_interests = []
		if category_interests is not None:
			for cat in category_interests:
				self._add_category_listener(cat)
		# add own uuid as identifier for hidden channel. (dlw)
		self._add_category_listener(str(self._uuid))

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
		'''Return (or create, store and return) a category listener.'''
		if iu_category not in self._listener_store:
			cat_listener = rsb.createListener(rsb.Scope("/ipaaca/category/"+str(iu_category)), config=self._participant_config)
			cat_listener.addHandler(self._handle_iu_events)
			self._listener_store[iu_category] = cat_listener
			self._category_interests.append(iu_category)
			logger.warning("Added listener in scope "+"/ipaaca/category/"+iu_category)#info

	def _handle_iu_events(self, event):
		'''Dispatch incoming IU events.

		Adds incoming IU's to the store, applies payload and commit updates to
		IU, calls IU event handlers.'

		Keyword arguments:
		event -- a converted RSB event
		'''
		type_ = type(event.data)
		if type_ is RemotePushIU:
			# a new IU
			if event.data.uid in self._iu_store:
				# already in our store
				pass
			else:
				self._iu_store[ event.data.uid ] = event.data
				event.data.buffer = self
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.ADDED, category=event.data.category)
		elif type_ is RemoteMessage:
			# a new Message, an ephemeral IU that is removed after calling handlers
			self._iu_store[ event.data.uid ] = event.data
			event.data.buffer = self
			self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.MESSAGE, category=event.data.category)
			del self._iu_store[ event.data.uid ]
		else:
			# an update to an existing IU
			if event.data.uid not in self._iu_store:
				logger.warning("Resend message for IU which we did not fully receive before.")
				# send resend request to remote server (dlw).
				remote_server = self._get_remote_server(event.data)
				resend_request = ipaaca_pb2.IUResendRequest()
				resend_request.uid = event.data.uid # target iu
				resend_request.hidden_name = str(self._uuid) # hidden channel name
				rRevision = remote_server.resendRequest(resend_request)
				if rRevision == 0:
					raise IUResendFailedError(self)

				return
			if type_ is ipaaca_pb2.IURetraction:
				# IU retraction (cannot be triggered remotely)
				iu = self._iu_store[event.data.uid]
				iu._revision = event.data.revision
				iu._apply_retraction() # for now - just sets the _rectracted flag.
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.RETRACTED, category=iu.category)
				# SPECIAL CASE: allow the handlers (which will need to find the IU
				#  in the buffer) to operate on the IU - then delete it afterwards!
				# FIXME: for now: retracted == deleted! Think about this later
				del(self._iu_store[iu.uid])
			else:
				if event.data.writer_name == self.unique_name:
					# Notify only for remotely triggered events;
					# Discard updates that originate from this buffer
					return
				#else:
				#	print('Got update written by buffer '+str(event.data.writer_name))

				if type_ is ipaaca_pb2.IUCommission:
					# IU commit
					iu = self._iu_store[event.data.uid]
					iu._apply_commission()
					iu._revision = event.data.revision
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.COMMITTED, category=iu.category)
				elif type_ is IUPayloadUpdate:
					# IU payload update
					iu = self._iu_store[event.data.uid]
					iu._apply_update(event.data)
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.UPDATED, category=iu.category)
				elif type_ is IULinkUpdate:
					# IU link update
					iu = self._iu_store[event.data.uid]
					iu._apply_link_update(event.data)
					self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.LINKSUPDATED, category=iu.category)
				else:
					logger.warning('Warning: _handle_iu_events failed to handle an object of type '+str(type_))

	def add_category_interests(self, category_interests):
		for interest in category_interests:
			self._add_category_listener(interest)


class OutputBuffer(Buffer):

	"""An OutputBuffer that holds local IUs."""

	def __init__(self, owning_component_name, participant_config=None):
		'''Create an Output Buffer.

		Keyword arguments:
		owning_component_name -- name of the entity that own this buffer
		participant_config -- RSB configuration
		'''
		super(OutputBuffer, self).__init__(owning_component_name, participant_config)
		self._unique_name = '/ipaaca/component/' + str(owning_component_name) + 'ID' + self._uuid + '/OB'
		self._server = rsb.createServer(rsb.Scope(self._unique_name))
		self._server.addMethod('updateLinks', self._remote_update_links, IULinkUpdate, int)
		self._server.addMethod('updatePayload', self._remote_update_payload, IUPayloadUpdate, int)
		self._server.addMethod('commit', self._remote_commit, ipaaca_pb2.IUCommission, int)
		# add method to trigger a resend request. (dlw)
		self._server.addMethod('resendRequest', self._remote_resend_request, ipaaca_pb2.IUResendRequest, int)
		self._informer_store = {}
		self._id_prefix = str(owning_component_name)+'-'+str(self._uuid)+'-IU-'
		self.__iu_id_counter_lock = threading.Lock()
		#self.__iu_id_counter = 0 # hbuschme: IUs now have their Ids assigned on creation

	def _create_own_name_listener(self, iu_category):
		# FIXME replace this
		'''Create an own name listener.'''
		#if iu_category in self._listener_store: return self._informer_store[iu_category]
		#cat_listener = rsb.createListener(rsb.Scope("/ipaaca/category/"+str(iu_category)), config=self._participant_config)
		#cat_listener.addHandler(self._handle_iu_events)
		#self._listener_store[iu_category] = cat_listener
		#self._category_interests.append(iu_category)
		#logger.info("Added category listener for "+iu_category)
		#return cat_listener
		pass

	# hbuschme: IUs now have their Ids assigned on creation
	#def _generate_iu_uid(self):
	#	'''Generate a unique IU id of the form ????'''
	#	with self.__iu_id_counter_lock:
	#		self.__iu_id_counter += 1
	#		number = self.__iu_id_counter
	#	return self._id_prefix + str(number)

	def _remote_update_links(self, update):
		'''Apply a remotely requested update to one of the stored IU's links.'''
		if update.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
			return 0
		iu = self._iu_store[update.uid]
		with iu.revision_lock:
			if (update.revision != 0) and (update.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				logger.warning("Remote write operation failed because request was out of date; IU "+str(update.uid))
				return 0
			if update.is_delta:
				iu.modify_links(add=update.new_links, remove=update.links_to_remove, writer_name=update.writer_name)
			else:
				iu.set_links(links=update.new_links, writer_name=update.writer_name)
			self.call_iu_event_handlers(update.uid, local=True, event_type=IUEventType.LINKSUPDATED, category=iu.category)
			return iu.revision

	def _remote_update_payload(self, update):
		'''Apply a remotely requested update to one of the stored IU's payload.'''
		if update.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
			return 0
		iu = self._iu_store[update.uid]
		with iu.revision_lock:
			if (update.revision != 0) and (update.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				logger.warning(u"Remote update_payload operation failed because request was out of date; IU "+str(update.uid))
				logger.warning(u"  Writer was: "+update.writer_name)
				logger.warning(u"  Requested update was: (New keys:) "+','.join(update.new_items.keys())+'  (Removed keys:) '+','.join(update.keys_to_remove))
				logger.warning(u"  Referred-to revision was "+str(update.revision)+' while local revision is '+str(iu.revision))
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
			self.call_iu_event_handlers(update.uid, local=True, event_type=IUEventType.UPDATED, category=iu.category)
			return iu.revision

	def _remote_resend_request(self, iu_resend_request_pack):
		''' Resend an requested iu. (dlw) '''
		if iu_resend_request_pack.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(iu_resend_request_pack.uid))
			return 0
		iu = self._iu_store[iu_resend_request_pack.uid]
		with iu.revision_lock:
			if (iu_resend_request_pack.hidden_name is not None) or (iu_resend_request_pack.hidden_name is not ""):
				informer = self._get_informer(iu_resend_request_pack.hidden_name)
				informer.publishData(iu)
				return iu.revision
			else:
				return 0

	def _remote_commit(self, iu_commission):
		'''Apply a remotely requested commit to one of the stored IUs.'''
		if iu_commission.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(iu_commission.uid))
			return 0
		iu = self._iu_store[iu_commission.uid]
		with iu.revision_lock:
			if (iu_commission.revision != 0) and (iu_commission.revision != iu.revision):
				# (0 means "do not pay attention to the revision number" -> "force update")
				logger.warning("Remote write operation failed because request was out of date; IU "+str(iu_commission.uid))
				return 0
			if iu.committed:
				return 0
			else:
				iu._internal_commit(writer_name=iu_commission.writer_name)
				self.call_iu_event_handlers(iu_commission.uid, local=True, event_type=IUEventType.COMMITTED, category=iu.category)
				return iu.revision

	def _get_informer(self, iu_category):
		'''Return (or create, store and return) an informer object for IUs of the specified category.'''
		if iu_category in self._informer_store:
			logger.info("Returning informer on scope "+"/ipaaca/category/"+str(iu_category))
			return self._informer_store[iu_category]
		informer_iu = rsb.createInformer(
				rsb.Scope("/ipaaca/category/"+str(iu_category)),
				config=self._participant_config,
				dataType=object)
		self._informer_store[iu_category] = informer_iu #new_tuple
		logger.info("Returning NEW informer on scope "+"/ipaaca/category/"+str(iu_category))
		return informer_iu #return new_tuple

	def add(self, iu):
		'''Add an IU to the IU store, assign an ID and publish it.'''
		# hbuschme: IUs now have their Ids assigned on creation
		#if iu._uid is not None:
		#	raise IUPublishedError(iu)
		#iu.uid = self._generate_iu_uid()
		if iu.uid in self._iu_store:
			raise IUPublishedError(iu)
		if iu.buffer is not None:
			raise IUPublishedError(iu)
		if iu.access_mode != IUAccessMode.MESSAGE:
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
					raise IUNotFoundError(iu_uid)
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
		link_update = IULinkUpdate(iu._uid, is_delta=is_delta, revision=revision)
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
		payload_update = IUPayloadUpdate(iu._uid, is_delta=is_delta, revision=revision)
		payload_update.new_items = new_items
		if is_delta:
			payload_update.keys_to_remove = keys_to_remove
		payload_update.writer_name = writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(payload_update)
		#print("  -- Sent update with writer name "+str(writer_name))


## --- RSB -------------------------------------------------------------------


def initialize_ipaaca_rsb():#{{{
	rsb.converter.registerGlobalConverter(
		IntConverter(wireSchema="int32", dataType=int))
	rsb.converter.registerGlobalConverter(
		IUConverter(wireSchema="ipaaca-iu", dataType=IU))
	rsb.converter.registerGlobalConverter(
		IUConverter(wireSchema="ipaaca-messageiu", dataType=Message))
	rsb.converter.registerGlobalConverter(
		IULinkUpdateConverter(
			wireSchema="ipaaca-iu-link-update",
			dataType=IULinkUpdate))
	rsb.converter.registerGlobalConverter(
		IUPayloadUpdateConverter(
			wireSchema="ipaaca-iu-payload-update",
			dataType=IUPayloadUpdate))
	rsb.converter.registerGlobalConverter(
		rsb.converter.ProtocolBufferConverter(
			messageClass=ipaaca_pb2.IUCommission))
	rsb.converter.registerGlobalConverter(
		rsb.converter.ProtocolBufferConverter(
			messageClass=ipaaca_pb2.IUResendRequest)) # dlw
	rsb.converter.registerGlobalConverter(
		rsb.converter.ProtocolBufferConverter(
			messageClass=ipaaca_pb2.IURetraction))
	rsb.__defaultParticipantConfig = rsb.ParticipantConfig.fromDefaultSources()
	#t = rsb.ParticipantConfig.Transport('spread', {'enabled':'true'})
	#rsb.__defaultParticipantConfig = rsb.ParticipantConfig.fromFile('rsb.cfg')
#}}}


## --- Module initialisation -------------------------------------------------

# register our own RSB Converters
initialize_ipaaca_rsb()

# Create a global logger for this module
logger = logging.getLogger('ipaaca')
logger.addHandler(IpaacaLoggingHandler(level=logging.INFO))
