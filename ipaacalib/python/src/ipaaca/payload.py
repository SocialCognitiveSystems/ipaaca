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


__all__ = [
	'Payload',
	'PayloadItemDictProxy',
	'PayloadItemListProxy',
]


_DEFAULT_PAYLOAD_UPDATE_TIMEOUT = 0.1


class Payload(dict):

	def __init__(self, iu, writer_name=None, new_payload=None, omit_init_update_message=False, update_timeout=_DEFAULT_PAYLOAD_UPDATE_TIMEOUT):
		self.iu = iu
		_pl = {}
		for k, v in ({} if new_payload is None else new_payload).iteritems():
			_pl[unicode(k, 'utf8') if type(k) == str else k] = unicode(v, 'utf8') if type(v) == str else v
		# NOTE omit_init_update_message is necessary to prevent checking for
		#   exceptions and sending updates in the case where we just receive
		#   a whole new payload from the remote side and overwrite it locally.
		for k, v in _pl.iteritems():
			dict.__setitem__(self, k, v)
		if (not omit_init_update_message) and (self.iu.buffer is not None):
			self.iu._modify_payload(
				is_delta=False,
				new_items=_pl,
				keys_to_remove=[],
				writer_name=writer_name)
		self._update_on_every_change = True
		self._update_timeout = update_timeout
		self._collected_modifications = {}
		self._collected_removals = []
		self._batch_update_writer_name = None  # name of remote buffer or None
		self._batch_update_lock = threading.RLock()
		self._batch_update_cond = threading.Condition(threading.RLock())

	def __getitem__(self, k):
		value = dict.__getitem__(self, k)
		if isinstance(value, dict):
			return PayloadItemDictProxy(value, self, k)
		elif isinstance(value, list):
			return PayloadItemListProxy(value, self, k)
		else:
			return value

	def __setitem__(self, k, v, writer_name=None):
		with self._batch_update_lock.acquire:
			for k, v in payload.iteritems():
				k = unicode(k, 'utf8') if type(k) == str else k
				v = unicode(v, 'utf8') if type(v) == str else v
			if self._update_on_every_change:
				self.iu._modify_payload(
					is_delta=True,
					new_items={k:v},
					keys_to_remove=[],
					writer_name=writer_name)
			else: # Collect additions/modifications
				self._batch_update_writer_name = writer_name
				self._collected_modifications[k] = v
			return dict.__setitem__(self, k, v)

	def __delitem__(self, k, writer_name=None):
		with self._batch_update_lock.acquire:
			k = unicode(k, 'utf8') if type(k) == str else k
			if self._update_on_every_change:
				self.iu._modify_payload(
					is_delta=True,
					new_items={},
					keys_to_remove=[k],
					writer_name=writer_name)
			else: # Collect additions/modifications
				self._batch_update_writer_name = writer_name
				self._collected_removals.append(k)
			return dict.__delitem__(self, k)

	# Context-manager based batch updates, not thread-safe (on remote updates)
	def __enter__(self):
		self._wait_batch_update_lock(self._update_timeout)
		self._update_on_every_change = False

	# Context-manager based batch updates, not thread-safe (on remote updates)
	def __exit__(self, type, value, traceback):
		self.iu._modify_payload(
			is_delta=True,
			new_items=self._collected_modifications,
			keys_to_remove=self._collected_removals,
			writer_name=self._batch_update_writer_name)
		self._collected_modifications = {}
		self._collected_removals = []
		self._update_on_every_change = True
		self._batch_update_writer_name = None
		self._batch_update_lock.release()

	def merge(self, payload, writer_name=None):
		with self._batch_update_lock.acquire:
			for k, v in payload.iteritems():
				k = unicode(k, 'utf8') if type(k) == str else k
				v = unicode(v, 'utf8') if type(v) == str else v
			self.iu._modify_payload(
				is_delta=True,
				new_items=payload,
				keys_to_remove=[],
				writer_name=writer_name)
			return dict.update(self, payload) # batch update

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


class PayloadItemProxy(object):

	def __init__(self, content, payload, identifier_in_payload):
		self.payload = payload
		self.content = content
		self.identifier_in_payload = identifier_in_payload

	def _notify_payload(self):
		self.payload[self.identifier_in_payload] = self.content

	def _create_proxy(self, obj, identifier_in_payload):
		if isinstance(obj, dict):
			return PayloadItemDictProxy(obj, self.payload, identifier_in_payload)
		elif isinstance(obj, list):
			return PayloadItemListProxy(obj, self.payload, identifier_in_payload)
		else:
			return obj

	def __setitem__(self, k, v):
		self.content.__setitem__(k,v)
		self._notify_payload()

	def __getitem__(self, k):
		item = self.content.__getitem__(k)
		return self._create_proxy(item, k)

	def __delitem__(self, k):
		self.content.__delitem__(k)
		self._notify_payload()


class PayloadItemDictProxy(PayloadItemProxy, dict):

	def __init__(self, content, payload, identifier_in_payload):
		dict.__init__(self, content)
		PayloadItemProxy.__init__(self, content, payload, identifier_in_payload)

	def clear(self):
		self.content.clear()
		self._notify_payload()

	def get(self, key, default=None):
		value = self.content.get(key, default)
		return self._create_proxy(value, key)

	def items(self):
		return [(key, value) for key, value in self.iteritems()]

	def iteritems(self):
		for key, value in self.content.iteritems():
			yield key, self._create_proxy(value, key)

	def values(self):
		return [value for value in self.itervalues()]

	def itervalues(self):
		for key, value in self.content.iteritems():
			yield self._create_proxy(value, key)

	def pop(self, key, *args):
		x = self.content.pop(key, *args)
		self._notify_payload()
		return x

	def popitem(self):
		x = self.content.popitem()
		self._notify_payload()
		return x

	def setdefault(self, key, default=None):
		notification_necessary = not key in self.content
		x = self.content.setdefault(key, default)
		if notification_necessary:
			self._notify_payload()
		return x

	def update(self, *args, **kwargs):
		self.content.update(*args, **kwargs)
		self._notify_payload()


class PayloadItemListProxy(PayloadItemProxy, list):

	def __init__(self, content, payload, identifier_in_payload):
		list.__init__(self, content)
		PayloadItemProxy.__init__(self, content, payload, identifier_in_payload)

	def __iter__(self):
		for index, item in enumerate(self.content):
			yield self._create_proxy(item, index)

	def append(self, x):
		self.content.append(x)
		self._notify_payload()

	def extend(self, l):
		self.content.extend(l)
		self._notify_payload()

	def insert(self, i, x):
		self.content.insert(i, x)
		self._notify_payload()

	def remove(self, x):
		self.content.remove(x)
		self._notify_payload()

	def pop(self, *args, **kwargs):
		x = self.content.pop(*args, **kwargs)
		self._notify_payload()
		return x

	def sort(self, cmp=None, key=None, reverse=False):
		self.content.sort(cmp, key, reverse)
		self._notify_payload()

	def reverse(self):
		self.content.reverse()
		self._notify_payload()
