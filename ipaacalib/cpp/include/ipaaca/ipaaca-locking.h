/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".
 *
 * Copyright (c) 2009-2015 Social Cognitive Systems Group
 *                         (formerly the Sociable Agents Group)
 *                         CITEC, Bielefeld University
 *
 * http://opensource.cit-ec.de/projects/ipaaca/
 * http://purl.org/net/ipaaca
 *
 * This file may be licensed under the terms of of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by the
 * Excellence Cluster EXC 277 Cognitive Interaction Technology.
 * The Excellence Cluster EXC 277 is a grant of the Deutsche
 * Forschungsgemeinschaft (DFG) in the context of the German
 * Excellence Initiative.
 */

#ifndef __ipaaca_locking_h_INCLUDED__
#define __ipaaca_locking_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif


/// a reentrant lock/mutex
#ifdef WIN32
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
IPAACA_HEADER_EXPORT class Lock
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT boost::recursive_mutex _mutex;
	public:
		IPAACA_HEADER_EXPORT inline Lock() {
		}
		IPAACA_HEADER_EXPORT inline ~Lock() {
		}
		IPAACA_HEADER_EXPORT inline void lock() {
			_mutex.lock();
		}
		IPAACA_HEADER_EXPORT inline void unlock() {
			_mutex.unlock();
		}
};
#else
#include <pthread.h>
IPAACA_HEADER_EXPORT class Lock
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT pthread_mutexattr_t _attrs;
		IPAACA_MEMBER_VAR_EXPORT pthread_mutex_t _mutex;
	public:
		IPAACA_HEADER_EXPORT inline Lock() {
			pthread_mutexattr_init(&_attrs);
			pthread_mutexattr_settype(&_attrs, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&_mutex, &_attrs);
		}
		IPAACA_HEADER_EXPORT inline ~Lock() {
			pthread_mutex_destroy(&_mutex);
			pthread_mutexattr_destroy(&_attrs);
		}
		IPAACA_HEADER_EXPORT inline void lock() {
			pthread_mutex_lock(&_mutex);
		}
		IPAACA_HEADER_EXPORT inline void unlock() {
			pthread_mutex_unlock(&_mutex);
		}
};
#endif

/// Stack-based lock holder. Create in a new stack frame
///  (i.e.  {}-block) and it will obtain the lock and
///  auto-release in on exiting the stack frame.
IPAACA_HEADER_EXPORT class Locker
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Lock* _lock;
	private:
		IPAACA_HEADER_EXPORT inline Locker(): _lock(NULL) { } // not available
	public:
		IPAACA_HEADER_EXPORT inline Locker(Lock& lock): _lock(&lock) {
			//std::cout << "-- Locker: lock" << std::endl;
			_lock->lock();
		}
		IPAACA_HEADER_EXPORT inline ~Locker() {
			//std::cout << "-- Locker: unlock" << std::endl;
			_lock->unlock();
		}
};

#ifndef WIN32
IPAACA_HEADER_EXPORT class PthreadMutexLocker
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT pthread_mutex_t* _lock;
	private:
		IPAACA_HEADER_EXPORT inline PthreadMutexLocker(): _lock(NULL) { } // not available
	public:
		IPAACA_HEADER_EXPORT inline PthreadMutexLocker(pthread_mutex_t* lock): _lock(lock) {
			if (!lock) throw Exception("PthreadMutexLocker got a NULL mutex!");
			pthread_mutex_lock(_lock);
		}
		IPAACA_HEADER_EXPORT inline ~PthreadMutexLocker() {
			pthread_mutex_unlock(_lock);
		}
};
#endif

#endif
