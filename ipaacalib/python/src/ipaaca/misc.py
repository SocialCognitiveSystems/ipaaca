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



class IpaacaLoggingHandler(logging.Handler):
	'''A logging handler that prints to stdout.'''

	def __init__(self, prefix='IPAACA', level=logging.NOTSET):
		logging.Handler.__init__(self, level)
		self._prefix = prefix

	def emit(self, record):
		meta = '[%s: %s] ' % (self._prefix, str(record.levelname))
		msg = str(record.msg.format(record.args))
		print(meta + msg)


class GenericNoLoggingHandler(logging.Handler):
	'''A logging handler that produces no output'''
	def emit(self, record): pass


def get_library_logger():
	'''Get ipaaca's library-wide logger object.'''
	return logging.getLogger(ipaaca.defaults.IPAACA_LOGGER_NAME)


__IPAACA_LOGGING_HANDLER = IpaacaLoggingHandler('IPAACA')
__GENERIC_NO_LOG_HANDLER = GenericNoLoggingHandler()

# By default, suppress library logging
# - for IPAACA
get_library_logger().addHandler(__GENERIC_NO_LOG_HANDLER)
# - for RSB
logging.getLogger('rsb').addHandler(__GENERIC_NO_LOG_HANDLER)


def enable_logging(level=None):
	'''Enable ipaaca's 'library-wide logging.'''
	ipaaca_logger = get_library_logger()
	ipaaca_logger.addHandler(__IPAACA_LOGGING_HANDLER)
	ipaaca_logger.removeHandler(__GENERIC_NO_LOG_HANDLER)
	ipaaca_logger.setLevel(level=level if level is not None else
		ipaaca.defaults.IPAACA_DEFAULT_LOGGING_LEVEL)


class IpaacaArgumentParser(argparse.ArgumentParser):

	class IpaacaDefaultChannelAction(argparse.Action):

		def __call__(self, parser, namespace, values, option_string=None):
			ipaaca.defaults.IPAACA_DEFAULT_CHANNEL = values

	class IpaacaLoggingLevelAction(argparse.Action):

		def __call__(self, parser, namespace, values, option_string=None):
			enable_logging(values)

	class IpaacaRSBLoggingLevelAction(argparse.Action):

		def __call__(self, parser, namespace, values, option_string=None):
			rsb_logger = logging.getLogger('rsb')
			rsb_logger.addHandler(IpaacaLoggingHandler('RSB'))
			rsb_logger.removeHandler(__GENERIC_NO_LOG_HANDLER)
			rsb_logger.setLevel(level=values)

	def __init__(self, prog=None, usage=None, description=None, epilog=None,
			parents=[], formatter_class=argparse.HelpFormatter,
			prefix_chars='-', fromfile_prefix_chars=None, 
			argument_default=None, conflict_handler='error', add_help=True):
		super(IpaacaArgumentParser, self).__init__(prog=prog, usage=usage,
			description=description, epilog=epilog, parents=parents,
			formatter_class=formatter_class, prefix_chars=prefix_chars,
			fromfile_prefix_chars=fromfile_prefix_chars,
			argument_default=argument_default, 
			conflict_handler=conflict_handler, add_help=add_help)

	def _add_ipaaca_lib_arguments(self):
		ipaacalib_group = self.add_argument_group('IPAACA library arguments')
		ipaacalib_group.add_argument(
			'--ipaaca-default-channel', 
			action=self.IpaacaDefaultChannelAction,
			default='default',
			metavar='NAME',
			dest='_ipaaca_default_channel_',
			help="specify default IPAACA channel name (default: 'default')")
		ipaacalib_group.add_argument(
			'--ipaaca-enable-logging', 
			action=self.IpaacaLoggingLevelAction,
			choices=['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG'],
			dest='_ipaaca_logging_level_',
			help="enable IPAACA logging with threshold")
		rsblib_group = self.add_argument_group('RSB library arguments')
		rsblib_group.add_argument(
			'--rsb-enable-logging', 
			action=self.IpaacaRSBLoggingLevelAction,
			choices=['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG'],
			dest='_ipaaca_rsb_enable_logging_',
			help="enable RSB logging with threshold")

	def parse_args(self, args=None, namespace=None):
		self._add_ipaaca_lib_arguments() # Add ipaaca-args just before parsing
		result = super(IpaacaArgumentParser, self).parse_args(args, namespace)
		# Delete ipaaca specific arguments (beginning with '_ipaaca' and 
		# ending with an underscore) from the resulting Namespace object.
		for item in dir(result):
			if item.startswith('_ipaaca') and item.endswith('_'):
				delattr(result, item)
		return result
