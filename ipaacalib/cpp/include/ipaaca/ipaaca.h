/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".
 *
 * Copyright (c) 2009-2013 Sociable Agents Group
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

#ifndef __IPAACA_H__
#define __IPAACA_H__

/// ipaaca/IU/RSB protocol major version number
#define IPAACA_PROTOCOL_VERSION_MAJOR         2
/// ipaaca/IU/RSB protocol minor version number
#define IPAACA_PROTOCOL_VERSION_MINOR         0

/// running release number of ipaaca-c++
#define IPAACA_CPP_RELEASE_NUMBER             12
/// date of last release number increment
#define IPAACA_CPP_RELEASE_DATE     "2015-01-15"

#ifndef __FUNCTION_NAME__
	#ifdef WIN32   // Windows
		#define __FUNCTION_NAME__   __FUNCTION__
	#else          // POSIX
		#define __FUNCTION_NAME__   __func__
	#endif
#endif

#ifdef WIN32
	#define IPAACA_SYSTEM_DEPENDENT_CLASS_NAME(c) "class "##c
#else
	#define IPAACA_SYSTEM_DEPENDENT_CLASS_NAME(c) c
#endif

#ifdef WIN32
	#if defined(ipaaca_EXPORTS)
		#define  IPAACA_EXPORT
		#define  IPAACA_HEADER_EXPORT __declspec(dllexport)
		#define  IPAACA_MEMBER_VAR_EXPORT
	#else
		#define  IPAACA_EXPORT
		#define  IPAACA_HEADER_EXPORT __declspec(dllimport)
		#define  IPAACA_MEMBER_VAR_EXPORT
	#endif
#else
	#define IPAACA_EXPORT
	#define IPAACA_HEADER_EXPORT
	#define IPAACA_MEMBER_VAR_EXPORT
#endif

#ifdef IPAACA_DEBUG_MESSAGES
#define IPAACA_INFO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- " << i << std::endl;
#define IPAACA_WARNING(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- WARNING: " << i << std::endl;
#define IPAACA_IMPLEMENT_ME std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- IMPLEMENT ME" << std::endl;
#define IPAACA_TODO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- TODO: " << i << std::endl;
#else
#define IPAACA_INFO(i) ;
#define IPAACA_WARNING(i) ;
#define IPAACA_IMPLEMENT_ME ;
#define IPAACA_TODO(i) ;
#endif

#ifdef IPAACA_EXPOSE_FULL_RSB_API
#include <rsc/runtime/TypeStringTools.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Event.h>
#include <rsb/ParticipantConfig.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>
#include <rsb/converter/Converter.h>
#include <rsb/rsbexports.h>
#endif


/// marking pure virtual functions for extra readability
#define _IPAACA_ABSTRACT_

/// value to return when reading nonexistant payload keys
#define IPAACA_PAYLOAD_DEFAULT_STRING_VALUE ""

// seconds until remote writes time out
#define IPAACA_REMOTE_SERVER_TIMEOUT 2.0


#include <iostream>

// for logger
#include <iomanip>

#ifdef WIN32
// for Windows
#include <time.h>
#else
// for Linux and OS X
#include <sys/time.h>
#endif

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/lexical_cast.hpp>
#endif

#include <ipaaca/ipaaca.pb.h>

#include <set>
#include <list>

namespace ipaaca {

typedef uint32_t revision_t;

/// Type of the IU event. Realized as an integer to enable bit masks for filters.
typedef uint32_t IUEventType;
#define IU_ADDED         1
#define IU_COMMITTED     2
#define IU_DELETED       4
#define IU_RETRACTED     8
#define IU_UPDATED      16
#define IU_LINKSUPDATED 32
#define IU_MESSAGE      64
/// Bit mask for receiving all events
#define IU_ALL_EVENTS  127

/// Convert an int event type to a human-readable string
IPAACA_HEADER_EXPORT inline std::string iu_event_type_to_str(IUEventType type)
{
	switch(type) {
		case IU_ADDED: return "ADDED";
		case IU_COMMITTED: return "COMMITTED";
		case IU_DELETED: return "DELETED";
		case IU_RETRACTED: return "RETRACTED";
		case IU_UPDATED: return "UPDATED";
		case IU_LINKSUPDATED: return "LINKSUPDATED";
		case IU_MESSAGE: return "MESSAGE";
		default: return "(NOT A KNOWN SINGLE IU EVENT TYPE)";
	}
}

/// IU access mode: PUSH means that updates are broadcast; REMOTE means that reads are RPC calls; MESSAGE means a fire-and-forget message
IPAACA_HEADER_EXPORT enum IUAccessMode {
	IU_ACCESS_PUSH,
	IU_ACCESS_REMOTE,
	IU_ACCESS_MESSAGE
};

class PayloadEntryProxy;
class Payload;
class IUInterface;
class IU;
class Message;
class RemotePushIU;
class IULinkUpdate;
class IUPayloadUpdate;
class IUStore;
class FrozenIUStore;
class Buffer;
class InputBuffer;
class OutputBuffer;

//class InputBufferRsbAdaptor;
//class OutputBufferRsbAdaptor;

class CallbackIUPayloadUpdate;
class CallbackIULinkUpdate;
class CallbackIUCommission;
class CallbackIUResendRequest;
class CallbackIURetraction;

class IUConverter;
class MessageConverter;
class IUPayloadUpdateConverter;
class IULinkUpdateConverter;
class IntConverter;


// BufferConfiguration
IPAACA_HEADER_EXPORT class BufferConfiguration
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _basename;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> _category_interests;
		IPAACA_MEMBER_VAR_EXPORT std::string _channel;
	public:
		IPAACA_HEADER_EXPORT inline BufferConfiguration(const std::string basename) { _basename = basename; }
		IPAACA_HEADER_EXPORT const std::string get_basename() const { return _basename; }
		IPAACA_HEADER_EXPORT const std::vector<std::string> get_category_interests() const { return _category_interests; }
		IPAACA_HEADER_EXPORT const std::string get_channel() const { return _channel; }
};

// BufferConfigurationBuilder
IPAACA_HEADER_EXPORT class BufferConfigurationBuilder: private BufferConfiguration
{
	public:
		IPAACA_HEADER_EXPORT inline BufferConfigurationBuilder(const std::string basename):BufferConfiguration(basename) {}
		IPAACA_HEADER_EXPORT inline void set_basename(const std::string& basename)
		{
			_basename = basename;
		}
		IPAACA_HEADER_EXPORT inline void add_category_interest(const std::string& category)
		{
			_category_interests.push_back(category);
		}
		IPAACA_HEADER_EXPORT inline void set_channel(const std::string& channel)
		{
			_channel = channel;
		}

		IPAACA_HEADER_EXPORT const BufferConfiguration& get_buffer_configuration() { return *this; }

};




/// generate a UUID as an ASCII string
IPAACA_HEADER_EXPORT std::string generate_uuid_string();

/// store for (local) IUs. TODO Stores need to be unified more
IPAACA_HEADER_EXPORT class IUStore: public std::map<std::string, boost::shared_ptr<IU> >
{
};
/// store for RemotePushIUs. TODO Stores need to be unified more
IPAACA_HEADER_EXPORT class RemotePushIUStore: public std::map<std::string, boost::shared_ptr<RemotePushIU> > // TODO genericize to all remote IU types
{
};

IPAACA_HEADER_EXPORT class Exception: public std::exception//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _description;
	public:
		IPAACA_HEADER_EXPORT inline Exception(const std::string& description=""): _description(description) { }
		IPAACA_HEADER_EXPORT inline ~Exception() throw() { }
		IPAACA_HEADER_EXPORT const char* what() const throw() {
			return _description.c_str();
		}
};//}}}
IPAACA_HEADER_EXPORT class Abort: public std::exception//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _description;
	public:
		IPAACA_HEADER_EXPORT inline Abort(const std::string& description=""): _description(description) { }
		IPAACA_HEADER_EXPORT inline ~Abort() throw() { }
		IPAACA_HEADER_EXPORT const char* what() const throw() {
			return _description.c_str();
		}
};//}}}

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

typedef std::set<std::string> LinkSet;
typedef std::map<std::string, LinkSet> LinkMap;
IPAACA_HEADER_EXPORT class SmartLinkMap {
	friend std::ostream& operator<<(std::ostream& os, const SmartLinkMap& obj);
	friend class IUInterface;
	friend class IU;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_HEADER_EXPORT const LinkSet& get_links(const std::string& key);
		IPAACA_HEADER_EXPORT const LinkMap& get_all_links();

	protected:
		IPAACA_MEMBER_VAR_EXPORT LinkMap _links;
		IPAACA_MEMBER_VAR_EXPORT static LinkSet empty_link_set;
		IPAACA_HEADER_EXPORT void _add_and_remove_links(const LinkMap& add, const LinkMap& remove);
		IPAACA_HEADER_EXPORT void _replace_links(const LinkMap& links);
};

IPAACA_MEMBER_VAR_EXPORT const LinkSet EMPTY_LINK_SET;
//const std::set<std::string> EMPTY_LINK_SET;

//typedef boost::function<void (const std::string&, bool, IUEventType, const std::string&)> IUEventHandlerFunction;
IPAACA_HEADER_EXPORT typedef boost::function<void (boost::shared_ptr<IUInterface>, IUEventType, bool)> IUEventHandlerFunction;

IPAACA_HEADER_EXPORT class IUEventHandler {
	protected:
		IPAACA_MEMBER_VAR_EXPORT IUEventHandlerFunction _function;
		IPAACA_MEMBER_VAR_EXPORT IUEventType _event_mask;
		IPAACA_MEMBER_VAR_EXPORT bool _for_all_categories;
		IPAACA_MEMBER_VAR_EXPORT std::set<std::string> _categories;
	protected:
		IPAACA_HEADER_EXPORT inline bool _condition_met(IUEventType event_type, const std::string& category)
		{
			return ((_event_mask&event_type)!=0) && (_for_all_categories || (_categories.count(category)>0));
		}
	public:
		IPAACA_HEADER_EXPORT IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category);
		IPAACA_HEADER_EXPORT IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		//void call(Buffer* buffer, const std::string& uid, bool local, IUEventType event_type, const std::string& category);
		IPAACA_HEADER_EXPORT void call(Buffer* buffer, boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	typedef boost::shared_ptr<IUEventHandler> ptr;
};

IPAACA_HEADER_EXPORT class Buffer { //: public boost::enable_shared_from_this<Buffer> {//{{{
	friend class IU;
	friend class RemotePushIU;
	friend class CallbackIUPayloadUpdate;
	friend class CallbackIULinkUpdate;
	friend class CallbackIUCommission;
	friend class CallbackIUResendRequest;
	protected:
		//Lock _handler_lock;
		IPAACA_MEMBER_VAR_EXPORT std::string _uuid;
		IPAACA_MEMBER_VAR_EXPORT std::string _basename;
		IPAACA_MEMBER_VAR_EXPORT std::string _unique_name;
		IPAACA_MEMBER_VAR_EXPORT std::string _id_prefix;
		IPAACA_MEMBER_VAR_EXPORT std::string _channel;
		IPAACA_MEMBER_VAR_EXPORT std::vector<IUEventHandler::ptr> _event_handlers;
	protected:
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name) = 0;


		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef") = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
//		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
		IPAACA_HEADER_EXPORT void _allocate_unique_name(const std::string& basename, const std::string& function);
		IPAACA_HEADER_EXPORT inline Buffer(const std::string& basename, const std::string& function) {
			_allocate_unique_name(basename, function);
			_channel = "default";
		}
		IPAACA_HEADER_EXPORT void call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	public:
		IPAACA_HEADER_EXPORT virtual inline ~Buffer() { }
		IPAACA_HEADER_EXPORT inline const std::string& unique_name() { return _unique_name; }
		IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask = IU_ALL_EVENTS, const std::string& category="");
		//_IPAACA_ABSTRACT_ virtual void add(boost::shared_ptr<IUInterface> iu) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual boost::shared_ptr<IUInterface> get(const std::string& iu_uid) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual std::set<boost::shared_ptr<IUInterface> > get_ius() = 0;

		IPAACA_HEADER_EXPORT inline const std::string& channel() { return _channel; }
};
//}}}

IPAACA_HEADER_EXPORT class OutputBuffer: public Buffer { //, public boost::enable_shared_from_this<OutputBuffer>  {//{{{
	friend class IU;
	friend class RemotePushIU;
	friend class OutputBufferRsbAdaptor;
	protected:
	protected:
		//OutputBufferRsbAdaptor _rsb;
		IPAACA_MEMBER_VAR_EXPORT IUStore _iu_store;
		IPAACA_MEMBER_VAR_EXPORT Lock _iu_id_counter_lock;
#ifdef IPAACA_EXPOSE_FULL_RSB_API
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::Informer<rsb::AnyType>::Ptr> _informer_store;
		IPAACA_MEMBER_VAR_EXPORT rsb::patterns::ServerPtr _server;
		IPAACA_HEADER_EXPORT rsb::Informer<rsb::AnyType>::Ptr _get_informer(const std::string& category);
#endif
	protected:
		// informing functions
		IPAACA_HEADER_EXPORT void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef");
IPAACA_HEADER_EXPORT void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

		IPAACA_HEADER_EXPORT void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef");
		IPAACA_HEADER_EXPORT void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name);
		IPAACA_HEADER_EXPORT void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name);
		// remote access functions
		// _remote_update_links(IULinkUpdate)
		// _remote_update_payload(IUPayloadUpdate)
		// _remote_commit(protobuf::IUCommission)
		IPAACA_HEADER_EXPORT void _publish_iu(boost::shared_ptr<IU> iu);

		IPAACA_HEADER_EXPORT void _retract_iu(boost::shared_ptr<IU> iu);
	protected:
		IPAACA_HEADER_EXPORT OutputBuffer(const std::string& basename, const std::string& channel="default");
		IPAACA_HEADER_EXPORT void _initialize_server();
	public:
		IPAACA_HEADER_EXPORT static boost::shared_ptr<OutputBuffer> create(const std::string& basename);
		IPAACA_HEADER_EXPORT ~OutputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT void add(boost::shared_ptr<IU> iu);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(const std::string& iu_uid);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(boost::shared_ptr<IU> iu);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
		IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius();
	typedef boost::shared_ptr<OutputBuffer> ptr;
};
//}}}

IPAACA_HEADER_EXPORT class InputBuffer: public Buffer { //, public boost::enable_shared_from_this<InputBuffer>  {//{{{
	friend class IU;
	friend class RemotePushIU;
	friend class InputBufferRsbAdaptor;
		//InputBufferRsbAdaptor _rsb;
#ifdef IPAACA_EXPOSE_FULL_RSB_API
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::ListenerPtr> _listener_store;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::patterns::RemoteServerPtr> _remote_server_store;
		IPAACA_MEMBER_VAR_EXPORT RemotePushIUStore _iu_store;  // TODO genericize
		IPAACA_HEADER_EXPORT rsb::patterns::RemoteServerPtr _get_remote_server(const std::string& unique_server_name);
		IPAACA_HEADER_EXPORT rsb::ListenerPtr _create_category_listener_if_needed(const std::string& category);
		IPAACA_HEADER_EXPORT void _handle_iu_events(rsb::EventPtr event);
		IPAACA_HEADER_EXPORT void _trigger_resend_request(rsb::EventPtr event);
#endif
	protected:
		IPAACA_HEADER_EXPORT inline void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef")
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_link_update() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name)
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_publish_iu_resend() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef")
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_payload_update() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_commission() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_resendrequest() should never be invoked")
		}
	protected:
		IPAACA_HEADER_EXPORT InputBuffer(const BufferConfiguration& bufferconfiguration);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::set<std::string>& category_interests);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);

		IPAACA_MEMBER_VAR_EXPORT bool triggerResend;
		IPAACA_HEADER_EXPORT void set_resend(bool resendActive);
		IPAACA_HEADER_EXPORT bool get_resend();

	public:
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const BufferConfiguration& bufferconfiguration);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::set<std::string>& category_interests);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::vector<std::string>& category_interests);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);
		IPAACA_HEADER_EXPORT ~InputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
		IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius();
	typedef boost::shared_ptr<InputBuffer> ptr;
};
//}}}

IPAACA_HEADER_EXPORT class IUPayloadUpdate {//{{{
	public:
		IPAACA_MEMBER_VAR_EXPORT std::string uid;
		IPAACA_MEMBER_VAR_EXPORT revision_t revision;
		IPAACA_MEMBER_VAR_EXPORT std::string writer_name;
		IPAACA_MEMBER_VAR_EXPORT bool is_delta;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> new_items;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> keys_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj);
	typedef boost::shared_ptr<IUPayloadUpdate> ptr;
};//}}}

IPAACA_HEADER_EXPORT class IULinkUpdate {//{{{
	public:
		IPAACA_MEMBER_VAR_EXPORT std::string uid;
		IPAACA_MEMBER_VAR_EXPORT revision_t revision;
		IPAACA_MEMBER_VAR_EXPORT std::string writer_name;
		IPAACA_MEMBER_VAR_EXPORT bool is_delta;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::set<std::string> > new_links;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::set<std::string> > links_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj);
	typedef boost::shared_ptr<IULinkUpdate> ptr;
};//}}}



IPAACA_HEADER_EXPORT class Initializer
{
	public:
		IPAACA_HEADER_EXPORT static void initialize_ipaaca_rsb_if_needed();
		IPAACA_HEADER_EXPORT static void initialize_updated_default_config();
		IPAACA_HEADER_EXPORT static bool initialized();
		IPAACA_HEADER_EXPORT static void dump_current_default_config();
	protected:
		IPAACA_MEMBER_VAR_EXPORT static bool _initialized;
};

IPAACA_HEADER_EXPORT class PayloadEntryProxy//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Payload* _payload;
		IPAACA_MEMBER_VAR_EXPORT std::string _key;
	public:
		IPAACA_HEADER_EXPORT PayloadEntryProxy(Payload* payload, const std::string& key);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const std::string& value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const char* value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(double value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(bool value);
		IPAACA_HEADER_EXPORT operator std::string();
		IPAACA_HEADER_EXPORT operator long();
		IPAACA_HEADER_EXPORT operator double();
		IPAACA_HEADER_EXPORT operator bool();
		IPAACA_HEADER_EXPORT std::string to_str();
		//long to_int() { return operator long(); ;
		IPAACA_HEADER_EXPORT long to_long();
		IPAACA_HEADER_EXPORT double to_float();
		IPAACA_HEADER_EXPORT bool to_bool();
		// getters
		IPAACA_HEADER_EXPORT template<typename T> T get(); // specializations below
		// setters
};
// Available interpretations of payload entries (or children thereof) below.
//  Usage of standard complex data structures (vector etc.) currently entails
//  casting all entries to a uniform type (a-priori choice: std::string).
IPAACA_HEADER_EXPORT template<> long PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> double PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> bool PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::string PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::vector<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::list<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::map<std::string, std::string> PayloadEntryProxy::get();

//}}}

IPAACA_HEADER_EXPORT class Payload//{{{
{
	friend std::ostream& operator<<(std::ostream& os, const Payload& obj);
	friend class IUInterface;
	friend class IU;
	friend class Message;
	friend class RemotePushIU;
	friend class RemoteMessage;
	friend class IUConverter;
	friend class MessageConverter;
	friend class CallbackIUPayloadUpdate;
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> _store;
		IPAACA_MEMBER_VAR_EXPORT boost::weak_ptr<IUInterface> _iu;
	protected:
		IPAACA_HEADER_EXPORT void initialize(boost::shared_ptr<IUInterface> iu);
		IPAACA_HEADER_EXPORT inline void _set_owner_name(const std::string& name) { _owner_name = name; }
		IPAACA_HEADER_EXPORT void _remotely_enforced_wipe();
		IPAACA_HEADER_EXPORT void _remotely_enforced_delitem(const std::string& k);
		IPAACA_HEADER_EXPORT void _remotely_enforced_setitem(const std::string& k, const std::string& v);
		IPAACA_HEADER_EXPORT void _internal_replace_all(const std::map<std::string, std::string>& new_contents, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_merge(const std::map<std::string, std::string>& contents_to_merge, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_set(const std::string& k, const std::string& v, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_remove(const std::string& k, const std::string& writer_name="");
	public:
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() { return _owner_name; }
		// access
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key);
		IPAACA_HEADER_EXPORT operator std::map<std::string, std::string>();
		IPAACA_HEADER_EXPORT inline void set(const std::map<std::string, std::string>& all_elems) { _internal_replace_all(all_elems); }
		IPAACA_HEADER_EXPORT inline void set(const std::string& k, const std::string& v) { _internal_set(k, v); }
		IPAACA_HEADER_EXPORT inline void merge(const std::map<std::string, std::string>& elems_to_merge) { _internal_merge(elems_to_merge); }
		IPAACA_HEADER_EXPORT inline void remove(const std::string& k) { _internal_remove(k); }
		IPAACA_HEADER_EXPORT std::string get(const std::string& k);
	typedef boost::shared_ptr<Payload> ptr;
};//}}}

IPAACA_HEADER_EXPORT class IUInterface {//{{{
	friend class IUConverter;
	friend class MessageConverter;
	friend std::ostream& operator<<(std::ostream& os, const IUInterface& obj);
	protected:
		IPAACA_HEADER_EXPORT IUInterface();
	public:
		IPAACA_HEADER_EXPORT inline virtual ~IUInterface() { }
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _uid;
		IPAACA_MEMBER_VAR_EXPORT revision_t _revision;
		IPAACA_MEMBER_VAR_EXPORT std::string _category;
		IPAACA_MEMBER_VAR_EXPORT std::string _payload_type; // default is "MAP"
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		IPAACA_MEMBER_VAR_EXPORT bool _committed;
		IPAACA_MEMBER_VAR_EXPORT bool _retracted;
		IPAACA_MEMBER_VAR_EXPORT IUAccessMode _access_mode;
		IPAACA_MEMBER_VAR_EXPORT bool _read_only;
		//boost::shared_ptr<Buffer> _buffer;
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
		IPAACA_MEMBER_VAR_EXPORT SmartLinkMap _links;
	protected:
		friend class Payload;
		// Internal functions that perform the update logic,
		//  e.g. sending a notification across the network
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name) = 0;
		//void _set_buffer(boost::shared_ptr<Buffer> buffer);
		IPAACA_HEADER_EXPORT void _associate_with_buffer(Buffer* buffer);
		IPAACA_HEADER_EXPORT void _set_buffer(Buffer* buffer);
		IPAACA_HEADER_EXPORT void _set_uid(const std::string& uid);
		IPAACA_HEADER_EXPORT void _set_owner_name(const std::string& owner_name);
	protected:
		// internal functions that do not emit update events
		IPAACA_HEADER_EXPORT inline void _add_and_remove_links(const LinkMap& add, const LinkMap& remove) { _links._add_and_remove_links(add, remove); }
		IPAACA_HEADER_EXPORT inline void _replace_links(const LinkMap& links) { _links._replace_links(links); }
	public:
		IPAACA_HEADER_EXPORT inline bool is_published() { return (_buffer != 0); }
		IPAACA_HEADER_EXPORT inline const std::string& uid() const { return _uid; }
		IPAACA_HEADER_EXPORT inline revision_t revision() const { return _revision; }
		IPAACA_HEADER_EXPORT inline const std::string& category() const { return _category; }
		IPAACA_HEADER_EXPORT const std::string& channel();
		IPAACA_HEADER_EXPORT inline const std::string& payload_type() const { return _payload_type; }
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() const { return _owner_name; }
		IPAACA_HEADER_EXPORT inline bool committed() const { return _committed; }
		IPAACA_HEADER_EXPORT inline IUAccessMode access_mode() const { return _access_mode; }
		IPAACA_HEADER_EXPORT inline bool read_only() const { return _read_only; }
		//inline boost::shared_ptr<Buffer> buffer() { return _buffer; }
		IPAACA_HEADER_EXPORT inline Buffer* buffer() const { return _buffer; }
		IPAACA_HEADER_EXPORT inline const LinkSet& get_links(std::string type) { return _links.get_links(type); }
		IPAACA_HEADER_EXPORT inline const LinkMap& get_all_links() { return _links.get_all_links(); }
		// Payload
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual Payload& payload() = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual const Payload& const_payload() const = 0;
		// setters
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void commit() = 0;
		// functions to modify and update links:
		IPAACA_HEADER_EXPORT void _publish_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

		IPAACA_HEADER_EXPORT void add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void set_links(const LinkMap& links, const std::string& writer_name = "");
		//    (with cpp specific convenience functions:)
		IPAACA_HEADER_EXPORT void add_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void remove_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
	typedef boost::shared_ptr<IUInterface> ptr;
};//}}}

IPAACA_HEADER_EXPORT class IU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class CallbackIUPayloadUpdate;
	friend class CallbackIULinkUpdate;
	friend class CallbackIUCommission;
	friend class CallbackIUResendRequest;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
	   IPAACA_MEMBER_VAR_EXPORT  Lock _revision_lock;
	protected:
		IPAACA_HEADER_EXPORT inline void _increase_revision_number() { _revision++; }
		IPAACA_HEADER_EXPORT IU(const std::string& category, IUAccessMode access_mode=IU_ACCESS_PUSH, bool read_only=false, const std::string& payload_type="MAP" );
	public:
		IPAACA_HEADER_EXPORT inline ~IU() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT static boost::shared_ptr<IU> create(const std::string& category, IUAccessMode access_mode=IU_ACCESS_PUSH, bool read_only=false, const std::string& payload_type="MAP" );
		IPAACA_HEADER_EXPORT inline Payload& payload() { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const { return _payload; }
		IPAACA_HEADER_EXPORT void commit();
	protected:
		IPAACA_HEADER_EXPORT virtual void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");

		IPAACA_HEADER_EXPORT virtual void _publish_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

		IPAACA_HEADER_EXPORT virtual void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		IPAACA_HEADER_EXPORT virtual void _internal_commit(const std::string& writer_name = "");
	public:
	typedef boost::shared_ptr<IU> ptr;
};//}}}
IPAACA_HEADER_EXPORT class Message: public IU {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class CallbackIUPayloadUpdate;
	friend class CallbackIULinkUpdate;
	friend class CallbackIUCommission;
	friend class CallbackIUResendRequest;
	protected:
		IPAACA_HEADER_EXPORT Message(const std::string& category, IUAccessMode access_mode=IU_ACCESS_MESSAGE, bool read_only=true, const std::string& payload_type="MAP" );
	public:
		IPAACA_HEADER_EXPORT inline ~Message() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT static boost::shared_ptr<Message> create(const std::string& category, IUAccessMode access_mode=IU_ACCESS_MESSAGE, bool read_only=true, const std::string& payload_type="MAP" );
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		IPAACA_HEADER_EXPORT void _internal_commit(const std::string& writer_name = "");
	public:
	typedef boost::shared_ptr<Message> ptr;
};//}}}

IPAACA_HEADER_EXPORT class RemotePushIU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
		IPAACA_HEADER_EXPORT RemotePushIU();
		IPAACA_HEADER_EXPORT static boost::shared_ptr<RemotePushIU> create();
	public:
		IPAACA_HEADER_EXPORT inline ~RemotePushIU() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT inline Payload& payload() { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const { return _payload; }
		IPAACA_HEADER_EXPORT void commit();
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		IPAACA_HEADER_EXPORT void _apply_update(IUPayloadUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_link_update(IULinkUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_commission();
		IPAACA_HEADER_EXPORT void _apply_retraction();
	typedef boost::shared_ptr<RemotePushIU> ptr;
};//}}}
IPAACA_HEADER_EXPORT class RemoteMessage: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
		IPAACA_HEADER_EXPORT RemoteMessage();
		IPAACA_HEADER_EXPORT static boost::shared_ptr<RemoteMessage> create();
	public:
		IPAACA_HEADER_EXPORT inline ~RemoteMessage() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT inline Payload& payload() { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const { return _payload; }
		IPAACA_HEADER_EXPORT void commit();
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		IPAACA_HEADER_EXPORT void _apply_update(IUPayloadUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_link_update(IULinkUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_commission();
		IPAACA_HEADER_EXPORT void _apply_retraction();
	typedef boost::shared_ptr<RemoteMessage> ptr;
};//}}}

IPAACA_HEADER_EXPORT class IUNotFoundError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUNotFoundError() throw() { }
		IPAACA_HEADER_EXPORT inline IUNotFoundError() { //boost::shared_ptr<IU> iu) {
			_description = "IUNotFoundError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUPublishedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUPublishedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUPublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUPublishedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUCommittedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUCommittedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUCommittedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUCommittedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUUpdateFailedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUUpdateFailedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUUpdateFailedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUUpdateFailedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUResendRequestFailedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUResendRequestFailedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUResendRequestFailedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUResendRequestFailedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUReadOnlyError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUReadOnlyError() throw() { }
		IPAACA_HEADER_EXPORT inline IUReadOnlyError() { //boost::shared_ptr<IU> iu) {
			_description = "IUReadOnlyError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyInABufferError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyInABufferError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyInABufferError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyInABufferError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUUnpublishedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUUnpublishedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUUnpublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUUnpublishedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyHasAnUIDError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyHasAnUIDError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyHasAnUIDError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnUIDError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyHasAnOwnerNameError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyHasAnOwnerNameError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyHasAnOwnerNameError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnOwnerNameError";
		}
};//}}}
IPAACA_HEADER_EXPORT class UUIDGenerationError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~UUIDGenerationError() throw() { }
		IPAACA_HEADER_EXPORT inline UUIDGenerationError() { //boost::shared_ptr<IU> iu) {
			_description = "UUIDGenerationError";
		}
};//}}}
IPAACA_HEADER_EXPORT class NotImplementedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~NotImplementedError() throw() { }
		IPAACA_HEADER_EXPORT inline NotImplementedError() { //boost::shared_ptr<IU> iu) {
			_description = "NotImplementedError";
		}
};//}}}

#ifdef IPAACA_EXPOSE_FULL_RSB_API
IPAACA_HEADER_EXPORT class CallbackIUPayloadUpdate: public rsb::patterns::Server::Callback<IUPayloadUpdate, int> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUPayloadUpdate(Buffer* buffer);
		IPAACA_HEADER_EXPORT boost::shared_ptr<int> call(const std::string& methodName, boost::shared_ptr<IUPayloadUpdate> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIULinkUpdate: public rsb::patterns::Server::Callback<IULinkUpdate, int> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIULinkUpdate(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int> call(const std::string& methodName, boost::shared_ptr<IULinkUpdate> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIUCommission: public rsb::patterns::Server::Callback<protobuf::IUCommission, int> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUCommission(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int> call(const std::string& methodName, boost::shared_ptr<protobuf::IUCommission> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIUResendRequest: public rsb::patterns::Server::Callback<protobuf::IUResendRequest, int> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUResendRequest(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int> call(const std::string& methodName, boost::shared_ptr<protobuf::IUResendRequest> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIURetraction: public rsb::patterns::Server::Callback<protobuf::IURetraction, int> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIURetraction(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int> call(const std::string& methodName, boost::shared_ptr<protobuf::IURetraction> update);
};//}}}

IPAACA_HEADER_EXPORT class IUConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IUConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class MessageConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT MessageConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class IUPayloadUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IUPayloadUpdateConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class IULinkUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IULinkUpdateConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class IntConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IntConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
#endif


// additional misc classes ( Command line options )//{{{
IPAACA_HEADER_EXPORT class CommandLineOptions {
	public:
		IPAACA_HEADER_EXPORT inline CommandLineOptions() { }
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> param_opts;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> param_set;
	public:
		IPAACA_HEADER_EXPORT void set_option(const std::string& name, bool expect, const char* optarg);
		IPAACA_HEADER_EXPORT std::string get_param(const std::string& o);
		IPAACA_HEADER_EXPORT bool is_set(const std::string& o);
		IPAACA_HEADER_EXPORT void dump();
	typedef boost::shared_ptr<CommandLineOptions> ptr;
};

class CommandLineParser {
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::map<char, std::string> longopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, char> shortopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> options; //  name / expect_param
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> defaults; // for opt params
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, int> set_flag; // for paramless opts
	protected:
		IPAACA_HEADER_EXPORT CommandLineParser();
	public:
		IPAACA_HEADER_EXPORT inline ~CommandLineParser() { }
		IPAACA_HEADER_EXPORT static inline boost::shared_ptr<CommandLineParser> create() {
			return boost::shared_ptr<CommandLineParser>(new CommandLineParser());
		}
		IPAACA_HEADER_EXPORT void initialize_parser_defaults();
		IPAACA_HEADER_EXPORT void dump_options();
		IPAACA_HEADER_EXPORT void add_option(const std::string& optname, char shortn, bool expect_param, const std::string& defaultv);
		IPAACA_HEADER_EXPORT void ensure_defaults_in( CommandLineOptions::ptr clo );
		IPAACA_HEADER_EXPORT CommandLineOptions::ptr parse(int argc, char* const* argv);
	typedef boost::shared_ptr<CommandLineParser> ptr;
};
//}}}

// additional misc functions ( String splitting / joining )//{{{
IPAACA_HEADER_EXPORT std::string str_join(const std::set<std::string>& set,const std::string& sep);
IPAACA_HEADER_EXPORT std::string str_join(const std::vector<std::string>& vec,const std::string& sep);
IPAACA_HEADER_EXPORT void str_split_wipe(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
IPAACA_HEADER_EXPORT void str_split_append(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
//}}}

// (snippets) //{{{
/*
class IUEventFunctionHandler: public rsb::EventFunctionHandler {
	protected:
		Buffer* _buffer;
	public:
		inline IUEventFunctionHandler(Buffer* buffer, const EventFunction& function, const std::string& method="")
			: EventFunctionHandler(function, method), _buffer(buffer) { }
};
*/
//}}}

IPAACA_MEMBER_VAR_EXPORT Lock& logger_lock();

#ifdef WIN32
#define LOG_IPAACA_CONSOLE(msg) { ipaaca::Locker logging_locker(ipaaca::logger_lock()); std::time_t result = std::time(NULL); std::cout << "[LOG] " << std::asctime(std::localtime(&result)) << " : " << msg << std::endl; }
#else
// use normal gettimeofday() on POSIX
#define LOG_IPAACA_CONSOLE(msg) { ipaaca::Locker logging_locker(ipaaca::logger_lock()); timeval logging_tim; gettimeofday(&logging_tim, NULL); double logging_t1=logging_tim.tv_sec+(logging_tim.tv_usec/1000000.0); std::cout << "[LOG] " << std::setprecision(15) << logging_t1 << " : " << msg << std::endl; }
#endif


} // of namespace ipaaca

#endif


