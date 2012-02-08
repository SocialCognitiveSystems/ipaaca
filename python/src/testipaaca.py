#!/usr/bin/env python

import time
import ipaaca
import sys

import unittest

class IpaacaLinksTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		self.ob.add(self.sensor_iu)
	def tearDown(self):
		pass
	def testSetLink(self):
		time.sleep(0.1)
		self.decision_iu = ipaaca.IU('decisioncategory')
		self.decision_iu.payload = {'data':'decision'}
		self.decision_iu.set_links( { 'grin': [self.sensor_iu.uid] } )
		self.ob.add(self.decision_iu)
		time.sleep(0.1)
		grinlinks = self.decision_iu.get_links('grin')
		self.assertIn(self.sensor_iu.uid, grinlinks)
		self.assertEqual(len(grinlinks), 1)

if __name__ == '__main__':
	unittest.main()

