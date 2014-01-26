#!/usr/bin/env python

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".	
#
# Copyright (c) 2009-2013 Sociable Agents Group
#                         CITEC, Bielefeld University	
#
# http://opensource.cit-ec.de/projects/ipaaca/
# http://purl.org/net/ipaaca
#
# This file may be licensed under the terms of of the
# GNU Lesser General Public License Version 3 (the ``LGPL''),
# or (at your option) any later version.
#
# Software distributed under the License is distributed
# on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the LGPL for the specific language
# governing rights and limitations.
#
# You should have received a copy of the LGPL along with this
# program. If not, go to http://www.gnu.org/licenses/lgpl.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.	
#
# The development of this software was supported by the
# Excellence Cluster EXC 277 Cognitive Interaction Technology.
# The Excellence Cluster EXC 277 is a grant of the Deutsche
# Forschungsgemeinschaft (DFG) in the context of the German
# Excellence Initiative.

import logging
import sys
import time
import ipaaca

color = False
max_size = 2048

def highlight_if_color(s, c='1'):
	return s if not color else '['+c+'m'+s+'[m'

def pretty_printed_iu_payload(iu):
	s='{ '
	for k,v in iu.payload.items():
		v2 = (('\''+v+'\'') if len(v)<=max_size else ('\''+v[:max_size]+'\'<excess length omitted>')).replace('\\','\\\\').replace('\n',highlight_if_color('\\n'))
		s += '\n' + '\t\t\'' + highlight_if_color(unicode(k),'1')+'\': '+unicode(v2)+', '
	s+=' }'
	return s

def event_type_color(typ):
	colors={'ADDED':'32;1', 'RETRACTED':'31;1', 'UPDATED':'33;1', 'MESSAGE':'34;1'}
	return '1' if typ not in colors else colors[typ]

def pretty_printed_iu_event(iu, event_type, local):
	s=''
	t=time.time()
	lt=time.localtime(t)
	s += highlight_if_color('%.3f'%t, '1')+' '+"%02d:%02d:%02d"%(lt.tm_hour, lt.tm_min, lt.tm_sec)
	s += ' '+highlight_if_color('%-9s'%event_type,event_type_color(event_type))+' category='+highlight_if_color(iu.category,event_type_color(event_type))+' uid='+iu.uid+' owner='+iu.owner_name+' payload='+pretty_printed_iu_payload(iu)
	return s

def my_update_handler(iu, event_type, local):
	t=time.localtime()
	print pretty_printed_iu_event(iu, event_type, local)

cats = []

keep_going=True
idx = 1
while keep_going:
	opt = sys.argv[idx] if idx<len(sys.argv) else None
	if opt=='--help':
		print('IU sniffer - usage:')
		print('  '+sys.argv[0]+' [--options] [<category1> [<category2 ...]]')
		print('         Listen to specified categories (default: all)')
		print('         Option --color : colorize output')
		print('         Option --size-limit <size> : limit payload display, chars (def: 2048)')
		sys.exit(0)
	elif opt=='--color':
		color = True
		idx += 1
	elif opt=='--size-limit':
		if len(sys.argv)<idx+2:
			print('Please specify a max size')
			sys.exit(1)
		max_size = int(sys.argv[idx+1])
		idx += 2
	else:
		cats = sys.argv[idx:]
		keep_going = False

ib = ipaaca.InputBuffer('SnifferIn', [''] if len(cats)==0 else cats)
ib.register_handler(my_update_handler)

print('')
print('Ipaaca IU Sniffer - run with --help to see options')
print('Listening for IU events of '+('any category...' if len(cats)==0 else 'categories: '+' '.join(cats)))
print('')
while True:
	time.sleep(1)

