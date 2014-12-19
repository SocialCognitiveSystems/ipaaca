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


import logging


__all__ = [
	'enum'
	'logger'
]


def enum(*sequential, **named):
	"""Create an enum type.

	Based on suggestion of Alec Thomas on stackoverflow.com:
	http://stackoverflow.com/questions/36932/
		whats-the-best-way-to-implement-an-enum-in-python/1695250#1695250
	"""
	enums = dict(zip(sequential, range(len(sequential))), **named)
	return type('Enum', (), enums)


# Create a global logger for ipaaca
class IpaacaLoggingHandler(logging.Handler):

	def __init__(self, level=logging.DEBUG):
		logging.Handler.__init__(self, level)

	def emit(self, record):
		meta = '[ipaaca] (%s) ' % str(record.levelname)
		msg = str(record.msg.format(record.args))
		print(meta + msg)

logger = logging.getLogger('ipaaca')
logger.addHandler(IpaacaLoggingHandler(level=logging.INFO))