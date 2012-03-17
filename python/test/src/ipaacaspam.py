#!/usr/bin/env python

import sys
import time
import unittest

import ipaaca

class Receiver(object):
	def __init__(self):
		self.count = 0
		self.last_time = 0
		self.ib = ipaaca.InputBuffer('PowerIn', ['spam'])
		self.ib.register_handler(self.handle_iu_event)
	
	def handle_iu_event(self, iu, event_type, local):
		if self.count==0: self.last_time=time.time()
		self.count+=1
		if self.count==1000:
			print "Received 1k IU updates at", int(1000.0/(time.time()-self.last_time)), "Hz"
			self.count=0
	
	def run(self):
		while True:
			time.sleep(1)


class Sender(object):
	def __init__(self, send_frequency):
		self.ob = ipaaca.OutputBuffer('PowerOut')
		self.iu = ipaaca.IU('spam')
		self.iu.payload = {'data':'0'}
		self.ob.add(self.iu)
		self.counter = 0
		self.frequency = send_frequency
		self.delay = 1.0/send_frequency
	
	def run(self):
		print "Sending with", self.delay ,"s delay - upper bound ",self.frequency,"Hz ..."
		last_time = time.time()
		while True:
			time.sleep(self.delay)
			self.counter += 1
			self.iu.payload['data'] = str(self.counter)
			if self.counter == 1000:
				print "Sent 1k updates at", int(1000.0/(time.time()-last_time)),"Hz"
				last_time = time.time()
				self.counter = 0
				#print ".",
				#sys.stdout.flush()
	
if __name__ == '__main__':
	if len(sys.argv)<2:
		print "specify either 'sender' or 'receiver' as an argument"
		print "   for the sender, you can additionally specify a delay (in 1/s)"
		print "   between 1 and 10000 please (=> 1 sec ... 0.0001 sec) [default: 1000 => 0.001 sec]"
		sys.exit(1)
	if sys.argv[1] == 'receiver':
		r = Receiver()
		r.run()
	elif sys.argv[1] == 'sender':
		freq=1000
		try:
			freq = int(sys.argv[2])
		except:
			pass
		if freq<1 or freq>10000:
			print "Between 1 and 10000 please (=> 1 sec ... 0.0001 sec)"
			sys.exit(1)
		s = Sender(send_frequency=freq)
		s.run()
	else:
		print "specify either 'sender' or 'receiver' as an argument"
		sys.exit(1)


