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

import sys
import time
import unittest

import ipaaca

RECV_CATEGORY = 'WORD'
SEND_CATEGORY = 'TEXT'


class TextSender(object):
	def __init__(self):
		self.ob = ipaaca.OutputBuffer('TextSenderOut')
		self.ob.register_handler(self.outbuffer_handle_iu_event)
		self.ib = ipaaca.InputBuffer('TextSenderIn', [RECV_CATEGORY])
		self.ib.register_handler(self.inbuffer_handle_iu_event)
	
	def outbuffer_handle_iu_event(self, iu, event_type, local):
		if event_type == "UPDATED":
			parent_uids = iu.get_links("GRIN")
			if len(parent_uids) > 0:
				parent_uid = list(parent_uids)[0]
				print "updating parent ..."
				next_uids = iu.get_links('SUCCESSOR')
				if len(next_uids) > 0:
					next_uid = list(next_uids)[0]
					next_iu = self.ob.iu_store[next_uid]
					next_letter_grin_links = next_iu.get_links("GRIN")
					if len(next_letter_grin_links) > 0 and list(next_letter_grin_links)[0] != parent_uid:
						# the next letter belongs to a new word
						parent_iu = self.ib.iu_store[parent_uid]
						parent_iu.payload['STATE'] = 'REALIZED'
					else:
						# the next letter belongs to the same word
						parent_iu = self.ib.iu_store[parent_uid]
						parent_iu.payload['STATE'] = 'STARTED'
				else:
					# there are no more letters, this is the end of the final word
					parent_iu = self.ib.iu_store[parent_uid]
					parent_iu.payload['STATE'] = 'REALIZED'
				print " ... done."
		else:
			print('(own IU event '+event_type+' '+str(iu.uid)+')')

	
	def inbuffer_handle_iu_event(self, iu, event_type, local):
		if event_type == "LINKSUPDATED":
			print "links updated"
		elif event_type == "ADDED": # and iu.category == RECV_CATEGORY:
			print("Received new word: "+iu.payload['WORD'])
			sender.publish_text_to_print(iu.payload['WORD'], parent_iu_uid=iu.uid)
		elif event_type == "RETRACTED":
			retracted_uid = iu.uid
			
		else:
			print('(IU event '+event_type+' '+str(iu.uid)+')')
		pass
	
	def find_last_iu(self):
		for iu in self.ob.iu_store.values():
			if len(iu.get_links('SUCCESSOR')) == 0:
				return iu
		return None
	
	def publish_text_to_print(self, text, parent_iu_uid=None):
		previous_iu = self.find_last_iu()
		if previous_iu is not None:
			# insert a blank if we already have words in the buffer
			iu = ipaaca.IU( SEND_CATEGORY )
			iu.payload = { 'CONTENT': ' ' }
			self.ob.add(iu)
			previous_iu.add_links( 'SUCCESSOR', [iu.uid] )
			iu.add_links( 'PREDECESSOR', [previous_iu.uid] )
			if parent_iu_uid is not None: iu.add_links( 'GRIN', [parent_iu_uid] )
			previous_iu = iu
		for c in text:
			iu = ipaaca.IU( SEND_CATEGORY )
			iu.payload = { 'CONTENT': c }
			self.ob.add(iu)
			if previous_iu is not None:
				previous_iu.add_links( 'SUCCESSOR', [iu.uid] )
				iu.add_links( 'PREDECESSOR', [previous_iu.uid] )
				if parent_iu_uid is not None: iu.add_links( 'GRIN', [parent_iu_uid] )
			if previous_iu is not None: print previous_iu.get_all_links()
			previous_iu = iu

if __name__ == '__main__':
	sender = TextSender()
	time.sleep(1.0)
	sender.publish_text_to_print('(INIT)')
	print "Press Ctrl-C to cancel..."
	while True:
		time.sleep(0.1)
	

