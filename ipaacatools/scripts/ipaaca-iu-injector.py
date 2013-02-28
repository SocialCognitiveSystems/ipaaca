#!/usr/bin/env python

import time
import logging
import ipaaca
import sys

def my_update_handler(iu, event_type, local):
	print(event_type+': '+str(iu))

if len(sys.argv)<2:
	print "Please use the program as follows:"
	print " "+sys.argv[0]+" <categoryname> [<payloadkey> <payloadvalue>] [<k2> <v2>] ..."
	sys.exit(1)

cate = sys.argv[1]
idx=2
pl={}
while len(sys.argv)>idx+1:
	pl[sys.argv[idx]] = sys.argv[idx+1]
	idx+=2

print "Sending Message of category "+cate
print " with payload "+str(pl)

ob = ipaaca.OutputBuffer('IUInjector')
ob.register_handler(my_update_handler)
iu_top = ipaaca.Message(cate)
iu_top.payload = pl
ob.add(iu_top)
print "Message sent."

time.sleep(0.3)
print "done."

