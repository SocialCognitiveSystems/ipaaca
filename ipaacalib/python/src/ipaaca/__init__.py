# -*- coding: utf-8 -*-

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".
#
# Copyright (c) 2009-2014 Social Cognitive Systems Group
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

import threading

import rsb
import rsb.converter

import ipaaca_pb2
import ipaaca.converter
from ipaaca.buffer import InputBuffer, OutputBuffer
from ipaaca.exception import *
from ipaaca.iu import IU, Message, IUAccessMode, IUEventType 
from ipaaca.misc import enable_logging, IpaacaArgumentParser
from ipaaca.payload import Payload


__RSB_INITIALIZER_LOCK = threading.Lock()
__RSB_INITIALIZED = False


def initialize_ipaaca_rsb_if_needed():
	'''Register own RSB Converters and initialise RSB from default config file.'''
	global __RSB_INITIALIZED
	with __RSB_INITIALIZER_LOCK:
		if __RSB_INITIALIZED:
			return
		else:
			rsb.converter.registerGlobalConverter(
				ipaaca.converter.IntConverter(
					wireSchema="int32",
					dataType=int))
		
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
					messageClass=ipaaca_pb2.IUCommission))
		
			rsb.converter.registerGlobalConverter(
				rsb.converter.ProtocolBufferConverter(
					messageClass=ipaaca_pb2.IUResendRequest))
		
			rsb.converter.registerGlobalConverter(
				rsb.converter.ProtocolBufferConverter(
					messageClass=ipaaca_pb2.IURetraction))
		
			rsb.__defaultParticipantConfig = rsb.ParticipantConfig.fromDefaultSources()

			__RSB_INITIALIZED = True
