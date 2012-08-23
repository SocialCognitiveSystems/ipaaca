#!/usr/bin/env python

import time
import logging
import ipaaca

def my_update_handler(iu, event_type, local):
	t=time.localtime()
	print str(t.tm_hour)+':'+str(t.tm_min)+':'+str(t.tm_sec),
	print(event_type+': '+unicode(iu))

ib = ipaaca.InputBuffer('SnifferIn', [''])
ib.register_handler(my_update_handler)

print("Listening for IU events of any category...")
print('')
while True:
	time.sleep(1)

