#!/usr/bin/env python

import sys
import time
import unittest

import ipaaca

SEND_CATEGORY = 'WORD'


class WordSender(object):
	def __init__(self):
		self.ob = ipaaca.OutputBuffer('WordSenderOut')
		self.ob.register_handler(self.outbuffer_handle_iu_event)
	
	def outbuffer_handle_iu_event(self, iu, event_type, local):
		if event_type == "UPDATED":
			print(iu.payload['WORD']+': '+iu.payload['STATE'])
		else:
			print('(own IU event '+event_type+' '+str(iu.uid)+')')
	
	def find_last_iu(self):
		for iu in self.ob.iu_store.values():
			if len(iu.get_links('SUCCESSOR')) == 0:
				return iu
		return None
	
	def publish_words(self, words):
		previous_iu = self.find_last_iu()
		for word in words:
			iu = ipaaca.IU( SEND_CATEGORY )
			iu.payload = { 'WORD': word }
			self.ob.add(iu)
			if previous_iu is not None:
				previous_iu.add_links( 'SUCCESSOR', [iu.uid] )
				iu.add_links( 'PREDECESSOR', [previous_iu.uid] )
			previous_iu = iu

if __name__ == '__main__':
	sender = WordSender()
	sender.publish_words(['this','is','a','demonstration','of','incremental','generation'])
	print sender.ob.unique_name
	print "Press Ctrl-C to cancel..."
	while True:
		time.sleep(0.1)
	

