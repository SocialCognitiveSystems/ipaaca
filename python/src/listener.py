#!/usr/bin/env python

import time
import logging
import ipaaca

iu_to_write = None

def my_update_handler(iu, event_type, local):
	global iu_to_write
	print(event_type+': '+str(iu))
	iu_to_write = iu



ib = ipaaca.InputBuffer('CoolReceiver', ['undef'])
ib.register_handler(my_update_handler)

counter = 0
#time.sleep(5)
while True:
	if iu_to_write is not None:
		try:
			counter += 1
			iu = iu_to_write
			#if counter == 1:
			#	iu.payload['a'] = 'remote'
			if counter % 3 == 1:
				iu.payload['a'] = 'REMOTELY SET '+str(counter)
			elif counter % 3 == 2:
				del iu.payload['a']
			else:
				iu.payload = {'a': 'reset'}
			
		except ipaaca.IUUpdateFailedError, e:
			ipaaca.logger.warning("Payload update failed (IU changed in the mean time)")
	time.sleep(0.1)

exit(0)
