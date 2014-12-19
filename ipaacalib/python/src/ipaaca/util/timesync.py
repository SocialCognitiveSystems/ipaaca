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

import time

import ipaaca.buffer
import ipaaca.iu


class TimesyncMaster(object):
	def __init__(self, component_name=None, timing_handler=None, debug_offset=0):
		self.ob = ipaaca.buffer.OutputBuffer(('' if component_name is None else component_name)+'TimesyncMaster')
		self.ib = ipaaca.buffer.InputBuffer(('' if component_name is None else component_name)+'TimesyncMaster', ['timesyncReply'])
		# component name to report (None => use buffer name)
		self.component_name = component_name if component_name is not None else self.ob.unique_name
		#
		self.ob.register_handler(self.handle_timesync_master)
		self.ib.register_handler(self.handle_timesync_master)
		# master_t1 is identical for all slaves
		self.master_t1 = None
		self.slave_t1s = {}
		self.master_t2s = {}
		self.slave_t2s = {}
		self.latencies = {}
		self.time_offsets = {}
		#
		self.debug_offset = debug_offset
		#
		self.timing_handler = timing_handler
	
	def set_timing_handler(self, timing_handler):
		self.timing_handler = timing_handler
	
	def send_master_timesync(self):
		iu = ipaaca.iu.IU('timesyncRequest')
		self.master_t1 = self.get_time()
		iu.payload = {
				'stage':'0',
				'master_t1':str(self.master_t1),
				'master':self.component_name,
				}
		self.ob.add(iu)
		print('Sent a stage 0 timesync as master '+self.component_name)
	
	def handle_timesync_master(self, iu, event_type, own):
		master = iu.payload['master']
		if not own and master == self.component_name:
			if self.component_name == master:
				# reply to our own initial IU
				slave = iu.payload['slave']
				stage = iu.payload['stage']
				if stage=='1':
					print('Received stage 1 from slave '+slave)
					# initial reply by slave
					t1 = iu.payload['slave_t1']
					self.slave_t1s[slave] = float(t1)
					t2 = self.master_t2s[slave] = self.get_time()
					iu.payload.merge({'master_t2': str(t2), 'stage':'2'})
					latency1 = t2 - self.master_t1
					self.latencies[slave] = latency1
					#print('Latency of round-trip 1: %.3f' % latency1)
				elif stage=='3':
					print('Received stage 3 from slave '+slave)
					# second reply by slave
					t2 = iu.payload['slave_t2']
					self.slave_t2s[slave] = float(t2)
					t_final = self.get_time()
					latency1 = self.latencies[slave]
					latency2 = t_final - self.master_t2s[slave]
					latency = self.latencies[slave] = (latency1+latency2)/2.0
					offset1 = (self.slave_t1s[slave]-self.master_t1)-latency/2.0
					offset2 = (self.slave_t2s[slave]-self.master_t2s[slave])-latency/2.0
					offset = (offset1+offset2)/2.0
					iu.payload.merge({'stage':'4', 'latency': str(latency), 'offset':str(offset)})
					if self.timing_handler is None:
						print('Determined timing of timesync slave '+slave)
						print('  Avg round-trip latency: %.3f s'%latency)
						print('  Offset of their clock: %.3f s'%offset)
					else:
						self.timing_handler(self.component_name, slave, latency, offset)
				else:
					# other stages are handled by time slave handler
					pass
	
	def get_time(self):
		return time.time() + self.debug_offset


class TimesyncSlave(object):
	def __init__(self, component_name=None, timing_handler=None, debug_offset=0):
		self.ob = ipaaca.buffer.OutputBuffer(('' if component_name is None else component_name)+'TimesyncSlave')
		self.ib = ipaaca.buffer.InputBuffer(('' if component_name is None else component_name)+'TimesyncSlave', ['timesyncRequest'])
		# component name to report (None => use buffer name)
		self.component_name = component_name if component_name is not None else self.ib.unique_name
		self.ob.register_handler(self.handle_timesync_slave)
		self.ib.register_handler(self.handle_timesync_slave)
		#self.master_t1 = None
		#self.master_t2 = None
		#self.master = None
		self.latency = None
		self.my_iu = None
		#
		self.debug_offset = debug_offset
		#
		self.timing_handler = timing_handler
	
	def set_timing_handler(self, timing_handler):
		self.timing_handler = timing_handler
	
	def handle_timesync_slave(self, iu, event_type, own):
		master = iu.payload['master']
		stage = iu.payload['stage']
		if self.component_name != master:
			if not own:
				# reply only to IUs from others
				if stage=='0':
					#print('Received stage 0 from master '+master)
					# initial reply to master
					self.my_iu = ipaaca.iu.IU('timesyncReply')
					# TODO: add grounded_in link too?
					t1 = self.get_time()
					self.my_iu.payload = iu.payload
					self.my_iu.payload['slave'] = self.component_name
					self.my_iu.payload['slave_t1'] = str(t1)
					self.my_iu.payload['stage'] = '1'

					#self.my_iu.payload.merge({
					#		'slave':self.component_name,
					#		'slave_t1':str(t1),
					#		'stage':'1',
					#		})
					self.ob.add(self.my_iu)
			else:
				if stage=='2':
					#print('Received stage 2 from master '+master)
					t2 = self.get_time()
					self.my_iu.payload.merge({
							'slave_t2':str(t2),
							'stage':'3',
							})
				elif stage=='4':
					latency = float(iu.payload['latency'])
					offset = float(iu.payload['offset'])
					if self.timing_handler is None:
						print('Timesync master '+master+' determined our timing: ')
						print('  Avg round-trip latency: %.3f s'%latency)
						print('  Offset of our clock: %.3f s'%offset)
					else:
						self.timing_handler(master, self.component_name, latency, offset)
	def get_time(self):
		return time.time() + self.debug_offset
