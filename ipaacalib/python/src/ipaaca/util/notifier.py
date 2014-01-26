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

from __future__ import print_function, with_statement

import threading

import ipaaca
from ipaaca.util.timesync import *

NotificationState = ipaaca.enum(
		NEW = 'new',
		OLD = 'old',
		DOWN = 'down'
		)

class ComponentError(Exception):
	def __init__(self, msg):
		super(ComponentError, self).__init__(msg)

class ComponentNotifier(object):

	NOTIFY_CATEGORY = "componentNotify"
	SEND_CATEGORIES = "send_categories"
	RECEIVE_CATEGORIES = "recv_categories"
	STATE = "state"
	NAME = "name"
	FUNCTION = "function"

	def __init__(self, component_name, component_function, send_categories, receive_categories, out_buffer=None, in_buffer=None):
		self.component_name = component_name
		self.component_function = component_function
		self.send_categories = frozenset(send_categories)
		self.receive_categories = frozenset(receive_categories)
		self.in_buffer = in_buffer if in_buffer is not None else ipaaca.InputBuffer(component_name + 'Notifier')
		self.out_buffer = out_buffer if out_buffer is not None else ipaaca.OutputBuffer(component_name + 'Notifier')
		self.terminated = False
		self.initialized = False
		self.notification_handlers = []
		self.initialize_lock = threading.Lock()
		self.notification_handler_lock = threading.Lock()
		self.submit_lock = threading.Lock()
		# clock sync code, sync slave/master pair will be installed when launched
		self.timesync_slave = None
		self.timesync_master = None
		self.timesync_master_handlers = []
		self.timesync_slave_handlers = []

	def _submit_notify(self, is_new):
		with self.submit_lock:
			notify_iu = ipaaca.Message(ComponentNotifier.NOTIFY_CATEGORY)
			notify_iu.payload = {
					ComponentNotifier.NAME: self.component_name,
					ComponentNotifier.FUNCTION: self.component_function,
					ComponentNotifier.SEND_CATEGORIES: ",".join(self.send_categories),
					ComponentNotifier.RECEIVE_CATEGORIES:  ",".join(self.receive_categories),
					ComponentNotifier.STATE: NotificationState.NEW if is_new else NotificationState.OLD,
					}          
			self.out_buffer.add(notify_iu)
	
	def terminate(self):
		with self.submit_lock:
			if self.terminated: return
			self.terminated = True
			notify_iu = ipaaca.Message(ComponentNotifier.NOTIFY_CATEGORY)
			notify_iu.payload = {
					ComponentNotifier.NAME: self.component_name,
					ComponentNotifier.FUNCTION: self.component_function,
					ComponentNotifier.SEND_CATEGORIES: ",".join(self.send_categories),
					ComponentNotifier.RECEIVE_CATEGORIES:  ",".join(self.receive_categories),
					ComponentNotifier.STATE: NotificationState.DOWN,
					}          
			self.out_buffer.add(notify_iu)

	def _handle_iu_event(self, iu, event_type, local):
		if iu.payload[ComponentNotifier.NAME] == self.component_name:
			return
		with self.notification_handler_lock:
			for h in self.notification_handlers:
				h(iu, event_type, local)
		if iu.payload[ComponentNotifier.STATE] == "new":
			#print("submitting")
			self._submit_notify(False)

	def add_notification_handler(self, handler):
		with self.notification_handler_lock:
			self.notification_handlers.append(handler)

	def launch_timesync_slave_handlers(self, master, slave, latency, offset):
		for h in self.timesync_slave_handlers:
			h(master, slave, latency, offset)

	def launch_timesync_master_handlers(self, master, slave, latency, offset):
		for h in self.timesync_master_handlers:
			h(master, slave, latency, offset)

	def add_timesync_slave_handler(self, handler):
		self.timesync_slave_handlers.append(handler)
	
	def add_timesync_master_handler(self, handler):
		self.timesync_master_handlers.append(handler)
	
	def send_master_timesync(self):
		#if len(self.timesync_master_handlers)==0:
		#	print('Warning: Sending a master timesync without a registered result callback.')
		self.timesync_master.send_master_timesync()
	
	def initialize(self):
		with self.initialize_lock:
			if self.terminated:
				raise ComponentError('Attempted to reinitialize component '+component_name+' after termination')
			if (not self.initialized):
				self.timesync_slave = TimesyncSlave(component_name=self.component_name, timing_handler=self.launch_timesync_slave_handlers)
				self.timesync_master = TimesyncMaster(component_name=self.component_name, timing_handler=self.launch_timesync_master_handlers)
				self.in_buffer.register_handler(self._handle_iu_event, ipaaca.IUEventType.MESSAGE, ComponentNotifier.NOTIFY_CATEGORY)
				self._submit_notify(True)
				self.initialized = True
	
	def __enter__(self):
		self.initialize()
	
	def __exit__(self, t, v, tb):
		self.terminate()


