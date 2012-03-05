#!/usr/bin/env python

import sys
import time
import unittest

import hamcrest as hc
import ipaaca

def handle_iu_event(iu, event_type, local):
	#print('(IU event '+event_type+' '+str(iu.uid)+')')
	pass

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
		hc.assert_that(self.ib.iu_store, hc.has_key(self.sensor_iu.uid))
		self.assertEqual(len(self.ib.iu_store), 1)
	def testOutputBufferContents(self):
		hc.assert_that(self.ib.iu_store, hc.has_key(self.sensor_iu.uid))
		self.assertEqual(len(self.ob.iu_store), 1)

class IpaacaPayloadTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory', 'decisioncategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		self.ob.add(self.sensor_iu)
		
	def testPayloadContent(self):
		time.sleep(0.1)
		iu_received = self.ib.iu_store.get(self.sensor_iu.uid)
		self.assertEqual(iu_received.payload["data"], 'sensordata')


class IpaacaCommitTestCases(unittest.TestCase):

	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.iu = ipaaca.IU('sensorcategory')

	def testCommitBeforePublish(self):
		self.iu.commit()
		self.ob.add(self.iu)
		time.sleep(0.1)
		received_iu = self.ib.iu_store[self.iu.uid]
		self.assertTrue(received_iu.committed)

	def testCommitAfterPublish(self):
		self.ob.add(self.iu)
		self.iu.commit()
		time.sleep(0.1)
		received_iu = self.ib.iu_store[self.iu.uid]
		self.assertTrue(received_iu.committed)
	
	def testCommitAndLocalWrite(self):
		self.ob.add(self.iu)
		time.sleep(0.1)
		self.iu.commit()
		with self.assertRaises(ipaaca.IUCommittedError):
			self.iu.payload['data'] = 'updatedData'
	
	def testCommitAndRemoteWrite(self):
		self.ob.add(self.iu)
		time.sleep(0.1)
		self.iu.commit()
		received_iu = self.ib.iu_store[self.iu.uid]
		with self.assertRaises(ipaaca.IUCommittedError):
			received_iu.payload['data'] = 'updatedData'


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
		hc.assert_that(self.ib.iu_store, hc.has_key(self.decision_iu.uid))
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		hc.assert_that(grinlinks, hc.has_item(self.sensor_iu.uid))
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
		hc.assert_that(self.ib.iu_store, hc.has_key(self.decision_iu.uid))
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		self.assertEqual(len(grinlinks), 0)

class IpaacaCommitTestCase(unittest.TestCase):
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
	def testCommitAndLocalWrite(self):
		self.sensor_iu.commit()
		with self.assertRaises(ipaaca.IUCommittedError):
			self.sensor_iu.payload['data'] = 'updatedData'
	def testCommitAndRemoteWrite(self):
		self.sensor_iu.commit()
		received_iu = self.ib.iu_store[self.sensor_iu.uid]
		with self.assertRaises(ipaaca.IUCommittedError):
			received_iu.payload['data'] = 'updatedData'

class IpaacaRemoteWriteTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ib.register_handler(handle_iu_event)
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.iu = ipaaca.IU('sensorcategory')
		self.iu.payload = {'data': 'sensordata'}
		time.sleep(0.1)
		self.ob.add(self.iu)
		time.sleep(0.1)
	def tearDown(self):
		pass
	def testRemotePayloadChange(self):
		self.assertIn(self.iu.uid, self.ib.iu_store)
		received_iu = self.ib.iu_store[self.iu.uid]
		received_iu.payload['data'] = 'updatedData'
		time.sleep(0.1)
		self.assertEqual(self.iu.payload['data'], 'updatedData')
	def testRemotePayloadReplace(self):
		self.assertIn(self.iu.uid, self.ib.iu_store)
		received_iu = self.ib.iu_store[self.iu.uid]
		received_iu.payload = { 'key1': 'value1', 'key2': 'value2' }
		time.sleep(0.1)
		self.assertEqual(len(self.iu.payload), 2)
		self.assertEqual(self.iu.payload['key1'], 'value1')
		self.assertEqual(self.iu.payload['key2'], 'value2')


if __name__ == '__main__':
	unittest.main()

