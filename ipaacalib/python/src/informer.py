#!/usr/bin/env python

import time
import ipaaca

def remote_change_dumper(iu, event_type, local):
	if local:
		print 'remote side '+event_type+': '+str(iu)


ob = ipaaca.OutputBuffer('CoolInformerOut')
ob.register_handler(remote_change_dumper)

iu_top = ipaaca.IU()
iu_top.payload = {'data': 'raw'}
ob.add(iu_top)

iu = ipaaca.IU()
iu.payload = {'a':'a1'}
ob.add(iu)

iu.payload = {'a':'a2', 'b':'b1'} #OK
del(iu.payload['b'])
iu.payload['c'] = 'c1'
iu.payload['a'] = 'a3'
iu.add_links('sameold', iu_top.uid)

time.sleep(1)
iu.commit()

while True:
	time.sleep(1)


