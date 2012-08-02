#!/usr/bin/env python

import time
import logging
import ipaaca

def my_update_handler(iu, event_type, local):
	print(event_type+': '+str(iu))

ib = ipaaca.InputBuffer('SnifferIn', [''])
ib.register_handler(my_update_handler)

while True:
	print(" .")
	time.sleep(1)

