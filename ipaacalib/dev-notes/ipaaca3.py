# -*- coding: utf-8 -*-

from __future__ import division, print_function

import abc
import collections
import re
import uuid

# defined in rsb.Scope.__COMPONENT_REGEX
RSB_SCOPE_COMPONENT_REGEX = re.compile(r'^[-_a-zA-Z0-9]+$')

def valid_ipaaca_name(name):
	'''Check if the given name (e.g., for buffer, category) is valid.
	'''
	return RSB_SCOPE_COMPONENT_REGEX.match(name)


class IpaacaError(Exception): pass


class IpaacaMetadataError(IpaacaError): pass


class IpaacaNameError(IpaacaError):
	def __init__(self, name, purpose):
		return '{name} is an invalid name for an IPAACA {purpose}.'.format(
				name=name,
				purpose=purpose)

class IUUpdateFailedError(IpaacaError): pass


class MetadataMixin(object):
	__metaclass__ = abc.ABCMeta

	@abc.abstractmethod
	def _set_defaults(self):
		return


class StandardMetadata(MetadataMixin):

	def _set_defaults(self):
		self._metadata['STD_COMMITTED'] = False
		self._metadata['STD_DELETED'] = False
		self._metadata['STD_READ_ONLY'] = False
		self._metadata['STD_RETRACTED'] = False

	@property
	def committed(self):
		return self._metadata['STD_COMMITTED']

	def commit(self):
		if not self.committed:
			self._metadata['STD_COMMITTED'] = True

	@property
	def live(self):
		return not (self._metadata['STD_DELETED'] or
				self._metadata['STD_READ_ONLY'] or
				self._metadata['STD_RETRACTED'])

	@property
	def read_only(self):
		return self._metadata['STD_READ_ONLY']

	@read_only.setter
	def read_only(self):
		if isinstance(self, Local):
			self._metadata['STD_READ_ONLY'] = True
		else:
			raise IpaacaError('Cannot set remote IU to read_only.') # TODO

	@property
	def retracted(self):
		return self._metadata['STD_RETRACTED']

	def retract(self):
		if isinstance(self, Local):
			self._metadata['STD_RETRACTED'] = True
		else:
			raise IpaacaError('Cannot retract remote IU.') # TODO


class AnnotationMetadata(MetadataMixin):

	def _set_defaults(self):
		self._metadata['T_START'] = 0
		self._metadata['T_END'] = 0

	@property
	def t_start(self):
		self._metadata['T_START']

	@t_start.setter
	def t_start(self, t):
		if t <= self._metadata['T_END']:
			self._metadata['T_START'] = t
		else:
			raise IpaacaMetadataError() #TODO

	@property
	def t_end(self):
		self._metadata['T_END']

	@t_end.setter
	def t_end(self, t):
		if t >= self._metadata['T_START']:
			self._metadata['T_END'] = t
		else:
			raise IpaacaMetadataError() #TODO

	@property
	def duration(self):
		return abs(self._metadata['T_END'] - self._metadata['T_START'])

	@property
	def label(self):
		self._payload[self._metadata['INTERVAL_LABEL_KEY']]


class AbstractIU(object):

	__metaclass__ = abc.ABCMeta

	def __init__(self, iu_id=None):
		self._iu_id = iu_id
		self._revision = 0
		self._buffer = None
		self._payload = None
		self._metadata = None
		self._links = None

	@property
	def buffer(self):
		return self._buffer

	def _set_buffer(self, buffer):
		if not self._buffer:
			self._buffer = buffer
		else:
			raise IpaacaError() # TODO: Write message

	@property
	def category(self):
		return self._category

	@property
	def id(self):
		return self._iu_id

	@property
	@abc.abstractmethod	
	def owner_name(self):
		return

	@property
	def revision(self):
		return self._revision

	def __enter__(self):
		pass
	
	def __exit__(self, type, value, traceback):
		pass


class Local(object):
	pass


class Remote(object):
	pass


class IU(AbstractIU, StandardMetadata):

	def __init__(self, category):
		super(IU, self).__init__(iu_id=str(uuid.uuid4()))
		if valid_ipaaca_name(category):
			self._category = category
		else:
			raise IpaacaNameError(category, 'category')
		self._buffer = None

	@property
	def owner_name(self):
		if self.buffer is not None:
			return self.buffer.id

	def retract(self):
		pass # TODO


class RemoteIU(AbstractIU): 

	def __init__(self):
		super(RemoteIU, self).__init__(self)
		self._owner_name = ''

	@property
	def owner_name(self):
		return self.owner_name

	def _set_owner_name(self, owner_name):
		if not self._owner_name:
			self._owner_name = owner_name
		else:
			raise IpaacaError() # TODO: Write message

	def commit(self):
		pass


class IUEventHandler(object):

	def __init__(self, handler_function, filter=None):
		self._handler_function = handler_function
		self._filter = filter

	def __call__(self, event):
		if self._filter(event):
			self._handler_function(
				#iu=iu,
				#event_type=event_type,
				#local=local
				)


class Buffer(object):

	def __init__(self, component_name):
		self._component_name = component_name
		self._iu_store = collections.OrderedDict()

	@property
	def component_name(self):
		return self._component_name


class InputBuffer(Buffer):

	def __init__(self, component_name):
		super(InputBuffer, self).__init__(component_name)


class OutputBuffer(Buffer):

	def __init__(self, component_name):
		super(OutputBuffer, self).__init__(component_name)

	def add(ius):
		if isinstance(ius, collections.Sequence):
			pass
		else:
			pass
