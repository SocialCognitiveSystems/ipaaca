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

def my_update_handler(iu, event_type, local):
	t=time.localtime()
	print "%02d:%02d:%02d"%(t.tm_hour, t.tm_min,t.tm_sec),
	print time.time(),
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

