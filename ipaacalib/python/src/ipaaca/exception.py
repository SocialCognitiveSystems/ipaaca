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


__all__ = [
	'IUCommittedError',
	'IUNotFoundError',
	'IUPayloadLockedError',
	'IUPayloadLockTimeoutError',
	'IUPublishedError',
	'IUReadOnlyError',
	'IUResendRequestFailedError',
	'IUUpdateFailedError',
]


class IUCommittedError(Exception):
	"""Error indicating that an IU is immutable because it has been committed to."""
	def __init__(self, iu):
		super(IUCommittedError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it has been committed to.')


class IUNotFoundError(Exception):
	"""Error indicating that an IU UID was unexpectedly not found in an internal store."""
	def __init__(self, iu_uid):
		super(IUNotFoundError, self).__init__('Lookup of IU ' + str(iu_uid) + ' failed.')


class IUPayloadLockTimeoutError(Exception):
	"""Error indicating that exclusive access to the Payload could not be obtained in time."""
	def __init__(self, iu):
		super(IUPayloadLockTimeoutError, self).__init__('Timeout while accessing payload of IU ' + str(iu.uid) + '.')


class IUPayloadLockedError(Exception):
	"""Error indicating that exclusive access to the Payload could not be obtained because someone actually locked it."""
	def __init__(self, iu):
		super(IUPayloadLockedError, self).__init__('IU '+str(iu.uid)+' was locked during access attempt.')


class IUPublishedError(Exception):
	"""Error publishing of an IU failed since it is already in the buffer."""
	def __init__(self, iu):
		super(IUPublishedError, self).__init__('IU ' + str(iu.uid) + ' is already present in the output buffer.')


class IUReadOnlyError(Exception):
	"""Error indicating that an IU is immutable because it is 'read only'."""
	def __init__(self, iu):
		super(IUReadOnlyError, self).__init__('Writing to IU ' + str(iu.uid) + ' failed -- it is read-only.')


class IUResendRequestFailedError(Exception):
	"""Error indicating that a remote IU resend failed."""
	def __init__(self, iu):
		super(IUResendFailedError, self).__init__('Remote resend failed for IU ' + str(iu.uid) + '.')


class IUUpdateFailedError(Exception):
	"""Error indicating that a remote IU update failed."""
	def __init__(self, iu):
		super(IUUpdateFailedError, self).__init__('Remote update failed for IU ' + str(iu.uid) + '.')
