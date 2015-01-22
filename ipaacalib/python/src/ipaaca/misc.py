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


import argparse
import logging

import ipaaca.defaults


__all__ = [
	'enum',
	'logger',
	'IpaacaArgumentParser',
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

	def __init__(self, level=logging.NOTSET):
		logging.Handler.__init__(self, level)

	def emit(self, record):
		meta = '[ipaaca] (%s) ' % str(record.levelname)
		msg = str(record.msg.format(record.args))
		print(meta + msg)


logger = logging.getLogger('ipaaca')
logger.addHandler(IpaacaLoggingHandler())
logger.setLevel(level=ipaaca.defaults.IPAACA_DEFAULT_LOGGING_LEVEL)



class IpaacaArgumentParser(argparse.ArgumentParser):

	class IpaacaDefaultChannelAction(argparse.Action):

		def __call__(self, parser, namespace, values, option_string=None):
			ipaaca.defaults.IPAACA_DEFAULT_CHANNEL = values

	class IpaacaLoggingLevelAction(argparse.Action):

		def __call__(self, parser, namespace, values, option_string=None):
			logger.setLevel(level=values)
	
	def __init__(self, prog=None, usage=None, description=None, epilog=None, parents=[], formatter_class=argparse.HelpFormatter, prefix_chars='-', fromfile_prefix_chars=None, argument_default=None, conflict_handler='error', add_help=True):
		super(IpaacaArgumentParser, self).__init__(prog=prog, usage=usage, description=description, epilog=epilog, parents=parents, formatter_class=formatter_class, prefix_chars=prefix_chars, fromfile_prefix_chars=fromfile_prefix_chars, argument_default=argument_default, conflict_handler=conflict_handler, add_help=add_help)

	def _add_ipaaca_lib_arguments(self):
		ipaacalib_group = self.add_argument_group(title='IPAACA library arguments''')
		ipaacalib_group.add_argument(
			'--ipaaca-default-channel', action=self.IpaacaDefaultChannelAction,
			default='default', metavar='NAME', dest='_ipaaca_default_channel_',
			help="IPAACA channel name which is used if a buffer does not define one locally (default: 'default')")
		ipaacalib_group.add_argument(
			'--ipaaca-logging-level', action=self.IpaacaLoggingLevelAction,
			choices=['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG'],
			dest='_ipaaca_logging_level_',
			help="IPAACA logging threshold (default: 'WARNING')")

	def parse_args(self, args=None, namespace=None):
		# Add ipaaca-library specific arguments at the very end
		self._add_ipaaca_lib_arguments()
		result = super(IpaacaArgumentParser, self).parse_args(args, namespace)
		for item in dir(result):
			if item.startswith('_ipaaca') and item.endswith('_'):
				delattr(result, item)
		return result


