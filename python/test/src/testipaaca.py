#!/usr/bin/env python

import time
import ipaaca
import sys

import unittest
	
def handle_iu_event(iu, event_type, local):
	print('(IU event '+event_type+' '+str(iu.uid)+')')

class IpaacaIUStoreTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ib.register_handler(handle_iu_event)
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		time.sleep(0.1)
		self.ob.add(self.sensor_iu)
		time.sleep(0.1)
	def tearDown(self):
		pass
	def testInputBufferContents(self):
		self.assertIn(self.sensor_iu.uid, self.ib.iu_store)
		self.assertEqual(len(self.ib.iu_store), 1)
	def testOutputBufferContents(self):
		self.assertIn(self.sensor_iu.uid, self.ob.iu_store)
		self.assertEqual(len(self.ob.iu_store), 1)

class IpaacaLinksTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory', 'decisioncategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		self.ob.add(self.sensor_iu)
	def tearDown(self):
		pass
	def testSetSingleLink(self):
		time.sleep(0.1)
		self.decision_iu = ipaaca.IU('decisioncategory')
		self.decision_iu.payload = {'data':'decision'}
		self.decision_iu.set_links( { 'grin': [self.sensor_iu.uid] } )
		self.ob.add(self.decision_iu)
		time.sleep(0.1)
		# test received version
		self.assertIn(self.decision_iu.uid, self.ib.iu_store)
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		self.assertIn(self.sensor_iu.uid, grinlinks)
		self.assertEqual(len(grinlinks), 1)
	def testSetAndRemoveSingleLink(self):
		time.sleep(0.1)
		self.decision_iu = ipaaca.IU('decisioncategory')
		self.decision_iu.payload = {'data':'decision'}
		self.decision_iu.set_links( { 'grin': [self.sensor_iu.uid] } )
		self.ob.add(self.decision_iu)
		time.sleep(0.1)
		self.decision_iu.remove_links('grin', [self.sensor_iu.uid])
		time.sleep(0.1)
		# test received version
		self.assertIn(self.decision_iu.uid, self.ib.iu_store)
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		self.assertEqual(len(grinlinks), 0)

if __name__ == '__main__':
	unittest.main()

