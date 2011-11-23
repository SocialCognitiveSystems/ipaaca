#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, division

import logging
import sys
import threading
import uuid

import rsb
import rsb.transport.converter

import ipaaca_pb2


__all__ = [
	'IUEventType',
	'IUAccessMode',
	'InputBuffer', 'OutputBuffer',
	'IU',
	'IUPublishedError', 'IUUpdateFailedError', 'IUCommittedError', 'IUReadOnlyError',
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
	protobuf_object.key = str(key)
	protobuf_object.value = str(value)
	protobuf_object.type = 'str' # TODO: more types


def unpack_typed_payload_item(protobuf_object):
	# TODO: more types
	return (protobuf_object.key, str(protobuf_object.value))


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
	UPDATED = 'UPDATED'
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


class IUCommittedError(Exception):
	"""Error indicating that an IU is immutable because it has been committed to."""
	def __init__(self, iu):
		super(IUCommittedError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it has been committed to.')


class IUReadOnlyError(Exception):
	"""Error indicating that an IU is immutable because it is 'read only'."""
	def __init__(self, iu):
		super(IUReadOnlyError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it is read-only.')


## --- Generation Architecture -----------------------------------------------


class LocalPayload(dict):#{{{
	
	"""Payload of IUs held locally, i.e., in an OutputBuffer."""
	
	def __init__(self, iu, writer_name=None, new_payload=None):
		"""Create local payload object.
		
		Keyword arguments:
		iu -- IU holding this payload
		writer_name -- @RAMIN: What is the reason to specify it here?
		new_payload -- a payload dictionary to initialise this object
			with, or None for an empty payload
		"""
		super(LocalPayload, self).__init__()
		self._iu = iu
		if new_payload is not None:
			for k, v in new_payload.items():
				dict.__setitem__(self, k, v)
		if self._iu.is_published:
			self._iu.buffer._send_iu_payload_update(
					self._iu,
					revision=iu.revision,
					is_delta=False,
					new_items=new_payload,
					keys_to_remove=[],
					writer_name = self._iu.owner_name if writer_name is None else writer_name)
	
	# @RAMIN: Not needed, right?
	#def __contains__(self, k):
	#	return dict.__contains__(self, k)
	
	#def __getitem__(self, k):
	#	return dict.__getitem__(self, k)
	
	def __setitem__(self, k, v, writer_name=None):
		"""Set an item from this payload locally and send update."""
		if self._iu.committed:
			raise IUCommittedError(self._iu)
		with self._iu.revision_lock:
			# set item locally
			result = dict.__setitem__(self, k, v)
			self._iu._increase_revision_number()
			if self._iu.is_published:
				# send update to remote holders
				self._iu.buffer._send_iu_payload_update(
						self._iu,
						revision=self._iu.revision,
						is_delta=True,
						new_items={k:v},
						keys_to_remove=[],
						writer_name=self._iu.owner_name if writer_name is None else writer_name)
			return result
	
	def __delitem__(self, k, writer_name=None):
		"""Delete an item from this payload locally and send update."""
		if self._iu.committed:
			raise IUCommittedError(self._iu)
		with self._iu.revision_lock:
			# delete item locally
			result = dict.__delitem__(self, k)
			self._iu._increase_revision_number()
			if self._iu.is_published:
				# send update to remote holders
				self._iu.buffer._send_iu_payload_update(
						self._iu,
						revision=self._iu.revision,
						is_delta=True,
						new_items={},
						keys_to_remove=[k],
						writer_name=self._iu.owner_name if writer_name is None else writer_name)
			return result
#}}}


class RemotePushPayload(dict):#{{{
	
	"""Payload of an IU of type 'PUSH' hold remotely, i.e., in an InputBuffer."""
	
	def __init__(self, remote_push_iu, new_payload):
		"""Create remote payload object.
		
		Keyword arguments:
		remote_push_iu -- remote IU holding this payload
		new_payload -- payload dict to initialise this remote payload with
		"""
		super(RemotePushPayload, self).__init__()
		self._remote_push_iu = remote_push_iu
		if new_payload is not None:
			for k,v in new_payload.items():
				dict.__setitem__(self, k, v)
	
	def __setitem__(self, k, v):
		"""Set item in this payload.
		
		Requests item setting from the OutputBuffer holding the local version
		of this IU. Returns when permission is granted and item is set;
		otherwise raises an IUUpdateFailedError.
		"""
		if self._remote_push_iu.committed:
			raise IUCommittedError(self._remote_push_iu)
		if self._remote_push_iu.read_only:
			raise IUReadOnlyError(self._remote_push_iu)
		requested_update = IUPayloadUpdate(
				uid=self._remote_push_iu.uid,
				revision=self._remote_push_iu.revision,
				is_delta=True,
				writer_name=self._remote_push_iu.buffer.unique_name,
				new_items={k:v},
				keys_to_remove=[])
		remote_server = self._remote_push_iu.buffer._get_remote_server(self._remote_push_iu)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise IUUpdateFailedError(self._remote_push_iu)
		else:
			self._remote_push_iu._revision = new_revision
			dict.__setitem__(self, k, v)
	
	def __delitem__(self, k):
		"""Delete item in this payload.
		
		Requests item deletion from the OutputBuffer holding the local version
		of this IU. Returns when permission is granted and item is deleted;
		otherwise raises an IUUpdateFailedError.
		"""
		if self._remote_push_iu.committed:
			raise IUCommittedError(self._remote_push_iu)
		if self._remote_push_iu.read_only:
			raise IUReadOnlyError(self._remote_push_iu)
		requested_update = IUPayloadUpdate(
				uid=self._remote_push_iu.uid,
				revision=self._remote_push_iu.revision,
				is_delta=True,
				writer_name=self._remote_push_iu.buffer.unique_name,
				new_items={},
				keys_to_remove=[k])
		remote_server = self._remote_push_iu.buffer._get_remote_server(self._remote_push_iu)
		new_revision = remote_server.updatePayload(requested_update)
		if new_revision == 0:
			raise IUUpdateFailedError(self._remote_push_iu)
		else:
			self._remote_push_iu._revision = new_revision
			dict.__delitem__(self, k)
	
	def _remotely_enforced_setitem(self, k, v):
		"""Sets an item when requested remotely."""
		return dict.__setitem__(self, k, v)
	
	def _remotely_enforced_delitem(self, k):
		"""Deletes an item when requested remotely."""
		return dict.__delitem__(self, k)

#}}}

class IUInterface(object):
	
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
		self._type = None
		self._owner_name = None
		self._committed = False
		self._access_mode = access_mode
		self._read_only = read_only
		self._buffer = None
		# payload is not present here
	
	def _get_revision(self):
		return self._revision
	revision = property(fget=_get_revision, doc='Revision number of the IU.')
	
	def _get_category(self):
		return self._category
	category = property(fget=_get_category, doc='Category of the IU.')
	
	def _get_type(self):
		return self._type
	type = property(fget=_get_type, doc='Type of the IU')
	
	def _get_committed(self):
		return self._committed
	committed = property(
			fget=_get_committed,
			doc='Flag indicating whether this IU has been committed to.')
	
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

	def __init__(self, access_mode=IUAccessMode.PUSH, read_only=False, category='undef', _type='undef'):
		super(IU, self).__init__(uid=None, access_mode=access_mode, read_only=read_only)
		self._revision = 1
		self._category = category
		self._type = _type
		self._payload = LocalPayload(iu=self)
		self.revision_lock = threading.Lock()
	
	def _increase_revision_number(self):
		self._revision += 1
	
	def _internal_commit(self, writer_name=None):
		if self.committed:
			raise IUCommittedError(self)
		with self.revision_lock:
			if not self._committed:
				self._increase_revision_number()
				self._committed = True
				self.buffer._send_iu_commission(self, writer_name=writer_name)
	
	def commit(self):
		"""Commit to this IU."""
		return self._internal_commit()
	
	def __str__(self):
		s = "IU{ "
		s += "uid="+self._uid+" "
		s += "(buffer="+(self.buffer.unique_name if self.buffer is not None else "<None>")+") "
		s += "owner_name=" + ("<None>" if self.owner_name is None else self.owner_name) + " "
		s += "payload={ "
		for k,v in self.payload.items():
			s += k+":'"+v+"', "
		s += "} "
		s += "}"
		return s
	
	def _get_payload(self):
		return self._payload
	def _set_payload(self, new_pl, writer_name=None):
		if self.committed:
			raise IUCommittedError(self)
		with self.revision_lock:
			self._increase_revision_number()
			self._payload = LocalPayload(
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
			doc='Unique ID of theIU.')

#}}}

class RemotePushIU(IUInterface):#{{{
	
	"""A remote IU with access mode 'PUSH'."""
	
	def __init__(self, uid, revision, read_only, owner_name, category, type, committed, payload):
		super(RemotePushIU, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only)
		self._revision = revision
		self._category = category
		self.owner_name = owner_name
		self._type = type
		self._committed = committed
		self._payload = RemotePushPayload(remote_push_iu=self, new_payload=payload)
	
	def __str__(self):
		s = "RemotePushIU{ "
		s += "uid="+self._uid+" "
		s += "(buffer="+(self.buffer.unique_name if self.buffer is not None else "<None>")+") "
		s += "owner_name=" + ("<None>" if self.owner_name is None else self.owner_name) + " "
		s += "payload={ "
		for k,v in self.payload.items():
			s += k+":'"+v+"', "
		s += "} "
		s += "}"
		return s
	
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
			self._payload = RemotePushPayload(remote_push_iu=self, new_payload=new_pl)
	payload = property(
			fget=_get_payload,
			fset=_set_payload,
			doc='Payload dictionary of the IU.')

	def _apply_update(self, update):
		"""Apply a IUPayloadUpdate to the IU."""
		self._revision = update.revision
		if update.is_delta:
			for k in update.keys_to_remove: self.payload._remotely_enforced_delitem(k)
			for k, v in update.new_items.items(): self.payload._remotely_enforced_setitem(k, v)
		else:
			# using '_payload' to circumvent the local writing methods
			self._payload = RemotePushPayload(remote_push_iu=self, new_payload=update.new_items)
	
	def _apply_commission(self):
		"""Apply commission to the IU"""
		self._committed = True
#}}}


class IntConverter(rsb.transport.converter.Converter):#{{{
	"""Convert Python int objects to Protobuf ints and vice versa."""
	def __init__(self, wireSchema="int", dataType=int):
		super(IntConverter, self).__init__(bytearray, dataType, wireSchema)
	
	def serialize(self, value):
		pbo = ipaaca_pb2.IntMessage()
		pbo.value = value
		return bytearray(pbo.SerializeToString()), self.wireSchema
	
	def deserialize(self, byte_stream, ws):
		pbo = ipaaca_pb2.IntMessage()
		pbo.ParseFromString( byte_stream )
		return pbo.value
#}}}


class IUConverter(rsb.transport.converter.Converter):#{{{
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
		pbo.type = iu._type
		pbo.owner_name = iu._owner_name
		pbo.committed = iu._committed
		pbo.access_mode = ipaaca_pb2.IU.PUSH # TODO
		pbo.read_only = iu._read_only
		for k,v in iu._payload.items():
			entry = pbo.payload.add()
			pack_typed_payload_item(entry, k, v)
		return bytearray(pbo.SerializeToString()), self.wireSchema
	
	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		if type == IU:
			pbo = ipaaca_pb2.IU()
			pbo.ParseFromString( byte_stream )
			if pbo.access_mode ==  ipaaca_pb2.IU.PUSH:
				_payload = {}
				for entry in pbo.payload:
					k, v = unpack_typed_payload_item(entry)
					_payload[k] = v
				remote_push_iu = RemotePushIU(
						uid=pbo.uid,
						revision=pbo.revision,
						read_only = pbo.read_only,
						owner_name = pbo.owner_name,
						category = pbo.category,
						type = pbo.type,
						committed = pbo.committed,
						payload=_payload
					)
				return remote_push_iu
			else:
				raise Exception("We can only handle IUs with access mode 'PUSH' for now!")
		else:
			raise ValueError("Inacceptable dataType %s" % type)
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
	# @RAMIN: Does this still need to be fixed? I guess not.
	# FIXME encode is required to use this class as an
	# argument for a remote server call
	
	#def encode(self, encoding):
	#	conv = IUPayloadUpdateConverter()
	#	return conv.serialize(self)
	
	def __str__(self):
		s =  'PayloadUpdate(' + 'uid=' + self.uid + ', '
		s += 'revision='+str(self.revision)+', '
		s += 'writer_name='+str(self.writer_name)+', '
		s += 'is_delta='+str(self.is_delta)+', '
		s += 'new_items = '+str(self.new_items)+', '
		s += 'keys_to_remove = '+str(self.keys_to_remove)+')'
		return s
#}}}


class IUPayloadUpdateConverter(rsb.transport.converter.Converter):#{{{
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
			pbo.ParseFromString( byte_stream )
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
		self._participant_config = participant_config #rsb.ParticipantConfig.fromDefaultSources() if participant_config is None else participant_config
		self._uuid = str(uuid.uuid4())[0:8]
		# Initialise with a temporary, but already unique, name
		self._unique_name = "undef-"+self._uuid
		self._iu_store = IUStore()
		self._iu_event_handlers = []
	
	def register_handler(self, handler_function, for_event_types=None, for_categories=None):
		"""Register a new IU event handler function.
		
		Keyword arguments:
		handler_function -- a function with the signature (IU, event_type, local)
		for_event_types -- a list of event types or None if handler should
			be called for all event types
		for_categories -- a list of category names or None if handler should
			be called for all categoires
		
		 """
		handler = IUEventHandler(handler_function=handler_function, for_event_types=for_event_types, for_categories=for_categories)
		self._iu_event_handlers.append(handler)
	
	def call_iu_event_handlers(self, uid, local, event_type, category):
		"""Call registered IU event handler functions registered for this event_type and category."""
		for h in self._iu_event_handlers:
			# print('calling an update handler for '+event_type+' -> '+str(h))
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
				self._create_category_listener_if_needed(cat)
	
	def _get_remote_server(self, iu):
		'''Return (or create, store and return) a remote server.'''
		if iu.owner_name in self._remote_server_store:
			return self._remote_server_store[iu.owner_name]
		#  TODO remove the str() when unicode is supported (issue #490)
		remote_server = rsb.createRemoteServer(rsb.Scope(str(iu.owner_name)))
		self._remote_server_store[iu.owner_name] = remote_server
		return remote_server
	
	def _create_category_listener_if_needed(self, iu_category):
		'''Return (or create, store and return) a category listener.'''
		if iu_category in self._listener_store: return self._informer_store[iu_category]
		cat_listener = rsb.createListener(rsb.Scope("/ipaaca/category/"+str(iu_category)), config=self._participant_config)
		cat_listener.addHandler(self._handle_iu_events)
		self._listener_store[iu_category] = cat_listener
		self._category_interests.append(iu_category)
		logger.info("Added category listener for "+iu_category)
		return cat_listener
	
	def _handle_iu_events(self, event):
		'''Dispatch incoming IU events.
		
		Adds incoming IU's to the store, applies payload and commit updates to
		IU, calls IU event handlers.'
		
		Keyword arguments:
		event -- a converted RSB event
		'''
		if type(event.data) is RemotePushIU:
			# a new IU
			if event.data.uid in self._iu_store:
				# already in our store
				pass
			else:
				self._iu_store[ event.data.uid ] = event.data
				event.data.buffer = self
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.ADDED, category=event.data.category)
		else:
			# an update to an existing IU
			if event.data.writer_name == self.unique_name:
				# Discard updates that originate from this buffer
				return
			if event.data.uid not in self._iu_store:
				# TODO: we should request the IU's owner to send us the IU
				logger.warning("Update message for IU which we did not fully receive before.")
				return
			if type(event.data) is ipaaca_pb2.IUCommission:
				# IU commit
				iu = self._iu_store[event.data.uid]
				iu._apply_commission()
				iu._revision = event.data.revision
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.COMMITTED, category=iu.category)
			elif type(event.data) is IUPayloadUpdate:
				# IU payload update
				iu = self._iu_store[event.data.uid]
				iu._apply_update(event.data)
				self.call_iu_event_handlers(event.data.uid, local=False, event_type=IUEventType.UPDATED, category=iu.category)


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
		self._server.addMethod('updatePayload', self._remote_update_payload, IUPayloadUpdate, int)
		self._server.addMethod('commit', self._remote_commit, ipaaca_pb2.IUCommission, int)
		self._informer_store = {}
		self._id_prefix = str(owning_component_name)+'-'+str(self._uuid)+'-IU-'
		self.__iu_id_counter_lock = threading.Lock()
		self.__iu_id_counter = 0
	
	def _generate_iu_uid(self):
		'''Generate a unique IU id of the form'''
		with self.__iu_id_counter_lock:
			self.__iu_id_counter += 1
			number = self.__iu_id_counter
		return self._id_prefix + str(number)

	def _remote_update_payload(self, update):
		'''Apply a remotely requested update to one of the stored IUs.'''
		if update.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
			return 0
		iu = self._iu_store[update.uid]
		if (update.revision != 0) and (update.revision != iu.revision):
			# (0 means "do not pay attention to the revision number" -> "force update")
			logger.warning("Remote write operation failed because request was out of date; IU "+str(update.uid))
			return 0
		if update.is_delta:
			for k in update.keys_to_remove:
				iu.payload.__delitem__(k, writer_name=update.writer_name)
			for k,v in update.new_items.items():
				iu.payload.__setitem__(k, v, writer_name=update.writer_name)
		else:
			iu._set_payload(update.new_items, writer_name=update.writer_name)
		self.call_iu_event_handlers(update.uid, local=True, event_type=IUEventType.UPDATED, category=iu.category)
		return iu.revision
	
	def _remote_commit(self, iu_commission):
		'''Apply a remotely requested commit to one of the stored IUs.'''
		if iu_commission.uid not in self._iu_store:
			logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(iu_commission.uid))
			return 0
		iu = self._iu_store[iu_commission.uid]
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
			return self._informer_store[iu_category]
		informer_iu = rsb.createInformer(
				rsb.Scope("/ipaaca/category/"+str(iu_category)),
				config=self._participant_config,
				dataType=object)
		self._informer_store[iu_category] = informer_iu #new_tuple
		logger.info("Added informer on "+iu_category)
		return informer_iu #return new_tuple
	
	def add(self, iu):
		'''Add an IU to the IU store, assign an ID and publish it.'''
		if iu._uid is not None:
			raise IUPublishedError(iu)
		iu.uid = self._generate_iu_uid()
		self._iu_store[iu._uid] = iu
		iu.buffer = self
		self._publish_iu(iu)
	
	def _publish_iu(self, iu):
		'''Publish an IU.'''
		informer = self._get_informer(iu._category)
		informer.publishData(iu)
	
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
		# print('sending IU commission event')
		informer = self._get_informer(iu._category)
		informer.publishData(iu_commission)
	
	def _send_iu_payload_update(self, iu, is_delta, revision, new_items=None, keys_to_remove=None, writer_name="undef"):
		'''Send an IU payload update.
		
		Keyword arguments:
		iu -- the IU being updated
		is_delta -- whether the update concerns only a single payload item or
			the whole payload dictionary
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


## --- RSB -------------------------------------------------------------------


def initialize_ipaaca_rsb():#{{{
	rsb.transport.converter.registerGlobalConverter(
		IntConverter(wireSchema="int32", dataType=int))
	rsb.transport.converter.registerGlobalConverter(
		IUConverter(wireSchema="ipaaca-iu", dataType=IU))
	rsb.transport.converter.registerGlobalConverter(
		IUPayloadUpdateConverter(
			wireSchema="ipaaca-iu-payload-update",
			dataType=IUPayloadUpdate))
	rsb.transport.converter.registerGlobalConverter(
		rsb.transport.converter.ProtocolBufferConverter(
			messageClass=ipaaca_pb2.IUCommission))
	rsb.__defaultParticipantConfig = rsb.ParticipantConfig.fromDefaultSources()
#}}}


## --- Module initialisation -------------------------------------------------

# register our own RSB Converters
initialize_ipaaca_rsb()

# Create a global logger for this module
logger = logging.getLogger('ipaaca')
logger.addHandler(IpaacaLoggingHandler())
