#!/usr/bin/env python

import time
import logging
import ipaaca
import sys

def my_update_handler(iu, event_type, local):
	t=time.localtime()
	print str(t.tm_hour)+':'+str(t.tm_min)+':'+str(t.tm_sec),
	print(event_type+': '+unicode(iu))

cats = []
if len(sys.argv)>1:
	cats = sys.argv[1:]

ib = ipaaca.InputBuffer('SnifferIn', [''] if len(cats)==0 else cats)
ib.register_handler(my_update_handler)

print("Listening for IU events of "+("any category..." if len(cats)==0 else "categories: "+' '.join(cats)))
print('')
while True:
	time.sleep(1)

