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


import time
import logging
import ipaaca

iu_to_write = None

def my_update_handler(iu, event_type, local):
	global iu_to_write
	print(event_type+': '+str(iu))
	iu_to_write = iu

ob = ipaaca.OutputBuffer('CoolListenerOut')

my_iu = ipaaca.IU()
my_iu.payload = {'some':'info'}
ob.add(my_iu)

ib = ipaaca.InputBuffer('CoolListenerIn', ['undef'])
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
			if counter == 10:
				iu.add_links('special', my_iu.uid)
			elif counter % 3 == 1:
				iu.payload['a'] = 'REMOTELY SET '+str(counter)
			elif counter % 3 == 2:
				del iu.payload['a']
			else:
				iu.payload = {'a': 'reset'}
			
		except ipaaca.IUUpdateFailedError, e:
			ipaaca.logger.warning("Payload update failed (IU changed in the mean time)")
	time.sleep(0.1)

exit(0)
