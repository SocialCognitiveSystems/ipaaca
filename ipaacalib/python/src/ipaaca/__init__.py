# -*- coding: utf-8 -*-

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".
#
# Copyright (c) 2009-2016 Social Cognitive Systems Group
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

from __future__ import division, print_function

import os
import threading

import rsb
import rsb.converter

import ipaaca.ipaaca_pb2
import ipaaca.converter
from ipaaca.buffer import InputBuffer, OutputBuffer
from ipaaca.exception import *
from ipaaca.iu import IU, Message, IUAccessMode, IUEventType 
from ipaaca.misc import enable_logging, IpaacaArgumentParser
from ipaaca.payload import Payload

#
# ipaaca.exit(int_retval)
#
from ipaaca.buffer import atexit_cleanup_function
def exit(int_retval=0):
	'''For the time being, this function can be used to
	circumvent any sys.exit blocks, while at the same time
	cleaning up the buffers (e.g. retracting IUs).
	
	Call once at the end of any python script (or anywhere
	in lieu of sys.exit() / os._exit(). '''
	print('ipaaca: cleaning up and exiting with code '+str(int_retval))
	atexit_cleanup_function()
	os._exit(int_retval)

__RSB_INITIALIZER_LOCK = threading.Lock()
__RSB_INITIALIZED = False


def initialize_ipaaca_rsb_if_needed():
	"""Initialise rsb if not yet initialise.

	   * Register own RSB onverters.
	   * Initialise RSB from enviroment variables, rsb config file, or
	     from default values for RSB trnasport, host, and port (via
	     ipaaca.defaults or ipaaca.misc.IpaacaArgumentParser).
	"""
	global __RSB_INITIALIZED
	with __RSB_INITIALIZER_LOCK:
		if __RSB_INITIALIZED:
			return
		else:
			#rsb.converter.registerGlobalConverter(
			#	ipaaca.converter.IntConverter(
			#		wireSchema="int32",
			#		dataType=int))
		
			rsb.converter.registerGlobalConverter(
				ipaaca.converter.IUConverter(
					wireSchema="ipaaca-iu",
					dataType=IU))
		
			rsb.converter.registerGlobalConverter(
				ipaaca.converter.MessageConverter(
					wireSchema="ipaaca-messageiu",
					dataType=Message))
		
			rsb.converter.registerGlobalConverter(
				ipaaca.converter.IULinkUpdateConverter(
					wireSchema="ipaaca-iu-link-update",
					dataType=converter.IULinkUpdate))
		
			rsb.converter.registerGlobalConverter(
				ipaaca.converter.IUPayloadUpdateConverter(
					wireSchema="ipaaca-iu-payload-update",
					dataType=converter.IUPayloadUpdate))
		
			rsb.converter.registerGlobalConverter(
				rsb.converter.ProtocolBufferConverter(
					messageClass=ipaaca.ipaaca_pb2.IUCommission))
		
			rsb.converter.registerGlobalConverter(
				rsb.converter.ProtocolBufferConverter(
					messageClass=ipaaca.ipaaca_pb2.IUResendRequest))
		
			rsb.converter.registerGlobalConverter(
				rsb.converter.ProtocolBufferConverter(
					messageClass=ipaaca.ipaaca_pb2.IURetraction))

			if ipaaca.defaults.IPAACA_DEFAULT_RSB_TRANSPORT is not None:
				if ipaaca.defaults.IPAACA_DEFAULT_RSB_TRANSPORT == 'spread':
					os.environ['RSB_TRANSPORT_SPREAD_ENABLED'] = str(1)
					os.environ['RSB_TRANSPORT_SOCKET_ENABLED'] = str(0)
				elif ipaaca.defaults.IPAACA_DEFAULT_RSB_TRANSPORT == 'socket':
					os.environ['RSB_TRANSPORT_SPREAD_ENABLED'] = str(0)
					os.environ['RSB_TRANSPORT_SOCKET_ENABLED'] = str(1)
					if ipaaca.defaults.IPAACA_DEFAULT_RSB_SOCKET_SERVER is not None:
						os.environ['RSB_TRANSPORT_SOCKET_SERVER'] = str(
							ipaaca.defaults.IPAACA_DEFAULT_RSB_SOCKET_SERVER)

			if ipaaca.defaults.IPAACA_DEFAULT_RSB_HOST is not None:
				os.environ['RSB_TRANSPORT_SPREAD_HOST'] = str(
					ipaaca.defaults.IPAACA_DEFAULT_RSB_HOST)
				os.environ['RSB_TRANSPORT_SOCKET_HOST'] = str(
					ipaaca.defaults.IPAACA_DEFAULT_RSB_HOST)
			if ipaaca.defaults.IPAACA_DEFAULT_RSB_PORT is not None:
				os.environ['RSB_TRANSPORT_SPREAD_PORT'] = str(
						ipaaca.defaults.IPAACA_DEFAULT_RSB_PORT)
				os.environ['RSB_TRANSPORT_SOCKET_PORT'] = str(
						ipaaca.defaults.IPAACA_DEFAULT_RSB_PORT)

			rsb.__defaultParticipantConfig = \
					rsb.ParticipantConfig.fromDefaultSources()

			__RSB_INITIALIZED = True
