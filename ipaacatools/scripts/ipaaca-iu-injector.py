#!/usr/bin/env python

import time
import logging
import ipaaca

def my_update_handler(iu, event_type, local):
	print(event_type+': '+str(iu))

ob = ipaaca.OutputBuffer('CoolInformerOut')
ob.register_handler(my_update_handler)

iu_top = ipaaca.IU('murmlfile')
iu_top.payload = {'data': 'HalloTest.us'}
#iu_top.payload = {'data': 'smile_7sec.us'}
ob.add(iu_top)

time.sleep(1)
print "Done"

