#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".
#
# Copyright (c) 2009-2015 Social Cognitive Systems Group
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

import datetime
import subprocess
import sys
import threading
import time
import traceback
import uuid

import ipaaca


__all__ = [
	'logger_send_ipaaca_logs',
	'logger_set_log_filename',
	'logger_set_module_name',
	'LOG_DEBUG',
	'LOG_INFO',
	'LOG_WARN',
	'LOG_WARNING',
	'LOG_ERROR',
]

LOGGER_LOCK = threading.RLock()

MODULE_NAME = sys.argv[0]

SEND_IPAACA_LOGS = True

OUTPUT_BUFFER = None

STANDARD_LOG_FILE_EXTENSION = '.log'

LOG_MODES = ['append', 'timestamp', 'overwrite']


def logger_set_log_filename(filename, existing=None):
	global OUTPUT_BUFFER
	if existing is not None and not existing in LOG_MODES:
		raise Exception('Invalid log mode {mode} given. '
			'Valid options are {options}.'.format(
				mode=existing,
				options=', '.join(LOG_MODES)))
	with LOGGER_LOCK:
		if OUTPUT_BUFFER is None:
			OUTPUT_BUFFER = ipaaca.OutputBuffer('LogSender')
		msg = ipaaca.Message('logcontrol')
		msg.payload = {
			'cmd': 'open_log_file',
			'filename': filename}
		if existing is not None:
			msg.payload['existing'] = existing
		OUTPUT_BUFFER.add(msg)


def logger_set_module_name(name):
	global MODULE_NAME
	with LOGGER_LOCK:
		MODULE_NAME = name


def logger_send_ipaaca_logs(flag=True):
	global SEND_IPAACA_LOGS
	with LOGGER_LOCK:
		SEND_IPAACA_LOGS = flag


def LOG_IPAACA(lvl, text, now=0.0, fn='???', thread='???'):
	global OUTPUT_BUFFER
	uid = str(uuid.uuid4())[0:8]
	with LOGGER_LOCK:
		if OUTPUT_BUFFER is None:
			OUTPUT_BUFFER = ipaaca.OutputBuffer('LogSender')
		msg = ipaaca.Message('log')
		msg.payload = {
				'module': MODULE_NAME,
				'function': fn,
				'level': lvl,
				'time':' %.3f'%now,
				'thread': thread,
				'uuid': uid,
				'text': text,}
		try:
			OUTPUT_BUFFER.add(msg)
		except Exception, e:
			LOG_ERROR('Caught an exception while logging via ipaaca. '
				+ ' str(e); ' 
				+ traceback.format_exc())


def LOG_CONSOLE(lvlstr, msg, fn_markup='[38;5;142m', msg_markup='', now=0.0, fn='???', thread='???'):
	if isinstance(msg, basestring):
		lines = msg.split('\n')
	else:
		lines = [msg]
	for line in lines:
		text = lvlstr+' '+thread+' '+fn_markup+fn+'[m'+' '+msg_markup+unicode(line)+'[m'
		print(text)
		fn = ' '*len(fn)


def LOG_ERROR(msg, now=None):
	now = time.time() if now is None else now
	f = sys._getframe(1)
	classprefix = (f.f_locals['self'].__class__.__name__+'.') if 'self' in f.f_locals else ''
	fn = classprefix + f.f_code.co_name
	thread = threading.current_thread().getName()
	with LOGGER_LOCK:
		if SEND_IPAACA_LOGS: LOG_IPAACA('ERROR', msg, now=now, fn=fn, thread=thread)
		LOG_CONSOLE('[38;5;9;1;4m[ERROR][m', msg, fn_markup='[38;5;203m', msg_markup='[38;5;9;1;4m', now=now, fn=fn, thread=thread)


def LOG_WARN(msg, now=None):
	now = time.time() if now is None else now
	f = sys._getframe(1)
	classprefix = (f.f_locals['self'].__class__.__name__+'.') if 'self' in f.f_locals else ''
	fn = classprefix + f.f_code.co_name
	thread = threading.current_thread().getName()
	with LOGGER_LOCK:
		if SEND_IPAACA_LOGS: LOG_IPAACA('WARN', msg, now=now, fn=fn, thread=thread)
		LOG_CONSOLE('[38;5;208;1m[WARN][m ', msg, fn_markup='[38;5;214m', msg_markup='[38;5;208;1m', now=now, fn=fn, thread=thread)


LOG_WARNING = LOG_WARN


def LOG_INFO(msg, now=None):
	now = time.time() if now is None else now
	f = sys._getframe(1)
	classprefix = (f.f_locals['self'].__class__.__name__+'.') if 'self' in f.f_locals else ''
	fn = classprefix + f.f_code.co_name
	thread = threading.current_thread().getName()
	with LOGGER_LOCK:
		if SEND_IPAACA_LOGS: LOG_IPAACA('INFO', msg, now=now, fn=fn, thread=thread)
		LOG_CONSOLE('[INFO] ', msg, now=now, fn=fn, thread=thread)


def LOG_DEBUG(msg, now=None):
	now = time.time() if now is None else now
	f = sys._getframe(1)
	classprefix = (f.f_locals['self'].__class__.__name__+'.') if 'self' in f.f_locals else ''
	fn = classprefix + f.f_code.co_name
	thread = threading.current_thread().getName()
	with LOGGER_LOCK:
		if SEND_IPAACA_LOGS: LOG_IPAACA('DEBUG', msg, now=now, fn=fn, thread=thread)
		LOG_CONSOLE('[2m[DEBUG][m', msg, fn_markup='[38;5;144m', msg_markup='[38;5;248m', now=now, fn=fn, thread=thread)


class LoggerComponent(object):

	def __init__(self, filename, log_mode='append'):
		self.ib = ipaaca.InputBuffer('Logger', ['log', 'logcontrol'])
		self.ib.register_handler(self._logger_handle_iu_event)
		self.logfile = None
		self.log_mode = log_mode
		self.open_logfile(filename)
		if self.logfile is None:
			print('Logging to console only ...')
		print('Press Ctrl-C at any time to terminate the logger.')

	def open_logfile(self, filename):
		with LOGGER_LOCK:
			if filename is None or filename.strip() == '':
				print('No log file name specified, not opening one.')
				self.close_logfile()
			else:
				new_logfile = None
				try:
					# create dir first
					ret = subprocess.call(
							'mkdir -p `dirname ' + filename + '`',
							shell=True)
					# proceed with dir+file
					if self.log_mode == 'timestamp':
						t = datetime.datetime.now().strftime('%Y-%m-%d-%H%M%S')
						if filename.endswith(STANDARD_LOG_FILE_EXTENSION):
							# insert timestamp between filename and extension
							# (only for standard extension)
							filename = filename.replace(
									STANDARD_LOG_FILE_EXTENSION,
									'.' + t + STANDARD_LOG_FILE_EXTENSION)
						else: # prepend timestamp
							filename = t + '_' + filename
					append_if_exist = not (self.log_mode == 'overwrite' or
							self.log_mode == 'timestamp')
					new_logfile = open(filename, 'a' if append_if_exist else 'w')
					if self.logfile is not None:
						text = u'Will now continue logging in log file ' + unicode(filename)
						uid = str(uuid.uuid4())[0:8]
						tim = time.time()
						record = {
							'uuid': uid, 
							'time': tim,
							'level': u'INFO',
							'text': text,
							'module': u'logger',
							'function': u'LoggerComponent.open_logfile', 
							'thread': '-', 
							'logreceivedtime': tim}
						self.logfile.write(unicode(record)+'\n')
						self.logfile.close()
					self.logfile = new_logfile
					print('Logging to console and {filename} ...'.format(filename=filename))
				except Exception as e:
					print('Failed to open logfile {filename} for writing! Keeping previous configuration'.format(filename=filename))

	def close_logfile(self):
		if self.logfile is not None:
			text = u'Closing of log file requested.'
			uid = unicode(uuid.uuid4())[0:8]
			tim = unicode(time.time())
			record = {
				'uuid': uid,
				'time': tim,
				'level': u'INFO',
				'text': text,
				'module': u'logger',
				'function': u'LoggerComponent.close_logfile', 
				'thread': u'-',
				'logreceivedtime': tim}
			self.logfile.write(unicode(record)+'\n')
			self.logfile.close()
			print('Closed current log file.')
			self.logfile = None

	def _logger_handle_iu_event(self, iu, event_type, own):
		received_time = "%.3f" % time.time()
		with LOGGER_LOCK:
			try:
				if iu.category == 'log':
					pl = iu.payload
					message = pl['text'] if 'text' in pl else '(No message.)'
					uid = '????????' if 'uuid' not in pl else pl['uuid']
					tim = '???' if 'time' not in pl else pl['time']
					module = '???' if 'module' not in pl else pl['module']
					function = '???' if 'function' not in pl else pl['function']
					thread = '???' if 'thread' not in pl else pl['thread']
					level = 'INFO' if 'level' not in pl else pl['level']
					# dump to console
					if level=='WARN':
						level='WARNING'
					if level not in ['DEBUG','INFO','WARNING','ERROR']:
						level = 'INFO'
					try:
						print('%s %-8s {%s} {%s} {%s} %s'%(tim, ('['+level+']'), thread, module, function, message))
					except:
						print('Failed to format a string for printing!')
					if self.logfile is not None:
						try:
							record = {
								'uuid': uid, 
								'time': tim, 
								'level': level, 
								'text': message,
								'module': module,
								'function': function, 
								'thread': thread,
								'logreceivedtime': received_time}
							self.logfile.write(unicode(record) + '\n')
						except:
							print('Failed to write to logfile!')
				elif iu.category == 'logcontrol':
					cmd = iu.payload['cmd']
					if cmd == 'open_log_file':
						filename = iu.payload['filename'] if 'filename' in iu.payload else ''
						if 'existing' in iu.payload:
							log_mode_ = iu.payload['existing'].lower()
							if log_mode_ not in LOG_MODES:
								LOG_WARN(u'Value of "existing" should be "append", timestamp, or  "overwrite", continuing with mode {mode}'.format(mode=self.log_mode))
							else:
								self.log_mode = log_mode_
						self.open_logfile(filename)
					elif cmd == 'close_log_file':
						self.close_logfile()
					else:
						LOG_WARN(u'Received unknown logcontrol command: '+unicode(cmd))
			except Exception, e:
				print('Exception while logging!')  # TODO write to file as well?
				print(u'  '+unicode(traceback.format_exc()))
