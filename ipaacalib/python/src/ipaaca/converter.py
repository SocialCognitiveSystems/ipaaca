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

import rsb.converter

import ipaaca_pb2
import ipaaca.defaults
import ipaaca.exception
import ipaaca.iu
import ipaaca.misc

LOGGER = ipaaca.misc.get_library_logger()

try:
	import simplejson as json
except ImportError:
	import json
	LOGGER.warn('INFO: Using module "json" instead of "simplejson". Install "simplejson" for better performance.')


__all__ = [
	'IntConverter',
	'IUConverter',
	'IULinkUpdate',
	'IULinkUpdateConverter',
	'IUPayloadUpdate',
	'IUPayloadUpdateConverter',
	'MessageConverter',
]


class IntConverter(rsb.converter.Converter):
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


def pack_payload_entry(entry, key, value, _type=ipaaca.iu.IUPayloadType.JSON):
	entry.key = key
	if _type == ipaaca.iu.IUPayloadType.JSON: 
		entry.value = json.dumps(value)
	elif _type == ipaaca.iu.IUPayloadType.STR or _type == 'MAP':
		entry.value = str(value)
	else:
		raise ipaaca.exception.IpaacaException('Asked to send payload entry with unsupported type "' +  _type + '".')
	entry.type = _type


def unpack_payload_entry(entry):
	# We assume that the only transfer types are 'STR' or 'JSON'. Both are transparently handled by json.loads
	if entry.type == ipaaca.iu.IUPayloadType.JSON:
		return json.loads(entry.value)
	elif entry.type == ipaaca.iu.IUPayloadType.STR or entry.type == 'str':
		return entry.value
	else:
		LOGGER.warn('Received payload entry with unsupported type "' + entry.type + '".')
		return entry.value


class IUConverter(rsb.converter.Converter):
	'''
	Converter class for Full IU representations
	wire:bytearray <-> wire-schema:ipaaca-full-iu <-> class ipaacaRSB.IU
	'''
	def __init__(self, wireSchema="ipaaca-iu", dataType=ipaaca.iu.IU):
		super(IUConverter, self).__init__(bytearray, dataType, wireSchema)
		self._access_mode = ipaaca_pb2.IU.PUSH
		self._remote_data_type = ipaaca.iu.RemotePushIU

	def serialize(self, iu):
		pbo = ipaaca_pb2.IU()
		pbo.access_mode = self._access_mode
		pbo.uid = iu._uid
		pbo.revision = iu._revision
		pbo.category = iu._category
		pbo.payload_type = iu._payload_type
		pbo.owner_name = iu._owner_name
		pbo.committed = iu._committed
		pbo.read_only = iu._read_only
		for k, v in iu._payload.iteritems():
			entry = pbo.payload.add()
			pack_payload_entry(entry, k, v, iu.payload_type)
		for type_ in iu._links.keys():
			linkset = pbo.links.add()
			linkset.type = type_
			linkset.targets.extend(iu._links[type_])
		return bytearray(pbo.SerializeToString()), self.wireSchema

	def deserialize(self, byte_stream, ws):
		pbo = ipaaca_pb2.IU()
		pbo.ParseFromString(str(byte_stream))
		_payload = {}
		for entry in pbo.payload:
			_payload[entry.key] = unpack_payload_entry(entry)
		_links = collections.defaultdict(set)
		for linkset in pbo.links:
			for target_uid in linkset.targets:
				_links[linkset.type].add(target_uid)
		return self._remote_data_type(
			uid=pbo.uid,
			revision=pbo.revision,
			read_only = pbo.read_only,
			owner_name = pbo.owner_name,
			category = pbo.category,
			payload_type = 'str' if pbo.payload_type is 'MAP' else pbo.payload_type,
			committed = pbo.committed,
			payload=_payload,
			links=_links)


class MessageConverter(IUConverter):
	'''
	Converter class for Full IU representations
	wire:bytearray <-> wire-schema:ipaaca-full-iu <-> class ipaacaRSB.IU
	'''
	def __init__(self, wireSchema="ipaaca-messageiu", dataType=ipaaca.iu.Message):
		super(MessageConverter, self).__init__(wireSchema, dataType)
		self._access_mode = ipaaca_pb2.IU.MESSAGE
		self._remote_data_type = ipaaca.iu.RemoteMessage


class IULinkUpdate(object):

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


class IULinkUpdateConverter(rsb.converter.Converter):
	
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
			LOGGER.debug('received an IULinkUpdate for revision '+str(pbo.revision))
			iu_link_up = IULinkUpdate( uid=pbo.uid, revision=pbo.revision, writer_name=pbo.writer_name, is_delta=pbo.is_delta)
			for entry in pbo.new_links:
				iu_link_up.new_links[str(entry.type)] = set(entry.targets)
			for entry in pbo.links_to_remove:
				iu_link_up.links_to_remove[str(entry.type)] = set(entry.targets)
			return iu_link_up
		else:
			raise ValueError("Inacceptable dataType %s" % type)


class IUPayloadUpdate(object):

	def __init__(self, uid, revision, is_delta, payload_type, writer_name="undef", new_items=None, keys_to_remove=None):
		super(IUPayloadUpdate, self).__init__()
		self.uid = uid
		self.revision = revision
		self.payload_type = payload_type
		self.writer_name = writer_name
		self.is_delta = is_delta
		self.new_items = {} if new_items is None else new_items
		self.keys_to_remove = [] if keys_to_remove is None else keys_to_remove

	def __str__(self):
		s =  'PayloadUpdate(' + 'uid=' + self.uid + ', '
		s += 'revision='+str(self.revision)+', '
		s += 'writer_name='+str(self.writer_name)+', '
		s += 'payload_type='+str(self.payload_type)+', '
		s += 'is_delta='+str(self.is_delta)+', '
		s += 'new_items = '+str(self.new_items)+', '
		s += 'keys_to_remove = '+str(self.keys_to_remove)+')'
		return s


class IUPayloadUpdateConverter(rsb.converter.Converter):
	def __init__(self, wireSchema="ipaaca-iu-payload-update", dataType=IUPayloadUpdate):
		super(IUPayloadUpdateConverter, self).__init__(bytearray, dataType, wireSchema)

	def serialize(self, iu_payload_update):
		pbo = ipaaca_pb2.IUPayloadUpdate()
		pbo.uid = iu_payload_update.uid
		pbo.writer_name = iu_payload_update.writer_name
		pbo.revision = iu_payload_update.revision
		for k, v in iu_payload_update.new_items.items():
			entry = pbo.new_items.add()
			pack_payload_entry(entry, k, v, iu_payload_update.payload_type)
		pbo.keys_to_remove.extend(iu_payload_update.keys_to_remove)
		pbo.is_delta = iu_payload_update.is_delta
		return bytearray(pbo.SerializeToString()), self.wireSchema

	def deserialize(self, byte_stream, ws):
		type = self.getDataType()
		if type == IUPayloadUpdate:
			pbo = ipaaca_pb2.IUPayloadUpdate()
			pbo.ParseFromString( str(byte_stream) )
			LOGGER.debug('received an IUPayloadUpdate for revision '+str(pbo.revision))
			iu_up = IUPayloadUpdate( uid=pbo.uid, revision=pbo.revision, payload_type=None, writer_name=pbo.writer_name, is_delta=pbo.is_delta)
			for entry in pbo.new_items:
				iu_up.new_items[entry.key] = unpack_payload_entry(entry)
			iu_up.keys_to_remove = pbo.keys_to_remove[:]
			return iu_up
		else:
			raise ValueError("Inacceptable dataType %s" % type)
