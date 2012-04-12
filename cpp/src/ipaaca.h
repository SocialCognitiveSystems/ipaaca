#ifndef __IPAACA_H__
#define __IPAACA_H_

#ifdef IPAACA_DEBUG_MESSAGES
#define IPAACA_INFO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- " << i << std::endl;
#define IPAACA_IMPLEMENT_ME std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- IMPLEMENT ME" << std::endl;
#define IPAACA_TODO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- TODO: " << i << std::endl;
#else
#define IPAACA_INFO(i) ;
#define IPAACA_IMPLEMENT_ME(i) ;
#define IPAACA_TODO(i) ;
#endif

// just for marking pure virtual functions for readability
#define _IPAACA_ABSTRACT_

#define IPAACA_PAYLOAD_DEFAULT_STRING_VALUE ""

#include <iostream>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>

#include <rsc/runtime/TypeStringTools.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Event.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>
#include <rsb/converter/Converter.h>
#include <rsb/rsbexports.h>

#include <ipaaca.pb.h>

#include <pthread.h>
#include <uuid/uuid.h>

//using namespace boost;
using namespace rsb;
using namespace rsb::filter;
using namespace rsb::converter;
using namespace rsb::patterns;

namespace ipaaca {

typedef uint32_t revision_t;

typedef uint32_t IUEventType;

#define IU_ADDED         1
#define IU_COMMITTED     2
#define IU_DELETED       4
#define IU_RETRACTED     8
#define IU_UPDATED      16
#define IU_LINKSUPDATED 32
//
#define IU_ALL_EVENTS   63

/*
enum IUEventType {
	IU_ADDED,
	IU_COMMITTED,
	IU_DELETED,
	IU_RETRACTED,
	IU_UPDATED,
	IU_LINKSUPDATED
};
*/

inline std::string iu_event_type_to_str(IUEventType type)
{
	switch(type) {
		case IU_ADDED: return "ADDED";
		case IU_COMMITTED: return "COMMITTED";
		case IU_DELETED: return "DELETED";
		case IU_RETRACTED: return "RETRACTED";
		case IU_UPDATED: return "UPDATED";
		case IU_LINKSUPDATED: return "LINKSUPDATED";
		default: return "(NOT A KNOWN SINGLE IU EVENT TYPE)";
	}
}

enum IUAccessMode {
	IU_ACCESS_PUSH,
	IU_ACCESS_REMOTE,
	IU_ACCESS_MESSAGE
};

//class {
//public:
//    template<typename T>
//    operator shared_ptr<T>() { return shared_ptr<T>(); }
//} NullPointer;

class PayloadEntryProxy;
class Payload;
class IUInterface;
class IU;
class RemotePushIU;
class IULinkUpdate;
class IULinkUpdateConverter;
class IUPayloadUpdate;
class IUPayloadUpdateConverter;
class IUStore;
class FrozenIUStore;
class Buffer;
class InputBuffer;
class OutputBuffer;

std::string generate_uuid_string();

class IUStore: public std::map<std::string, boost::shared_ptr<IU> >
{
};
class RemotePushIUStore: public std::map<std::string, boost::shared_ptr<RemotePushIU> > // TODO genericize to all remote IU types
{
};


class Lock
{
	protected:
		pthread_mutexattr_t _attrs;
		pthread_mutex_t _mutex;
	public:
		inline Lock() {
			pthread_mutexattr_init(&_attrs);
			pthread_mutexattr_settype(&_attrs, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&_mutex, &_attrs);
		}
		inline ~Lock() {
			pthread_mutex_destroy(&_mutex);
			pthread_mutexattr_destroy(&_attrs);
		}
		inline void lock() {
			pthread_mutex_lock(&_mutex);
		}
		inline void unlock() {
			pthread_mutex_unlock(&_mutex);
		}
};

typedef std::set<std::string> LinkSet;
typedef std::map<std::string, LinkSet> LinkMap;
class SmartLinkMap {
	friend std::ostream& operator<<(std::ostream& os, const SmartLinkMap& obj);
	friend class IUInterface;
	friend class IU;
	friend class IUConverter;
	public:
		const LinkSet& get_links(const std::string& key);
		const LinkMap& get_all_links();
	
	protected:
		LinkMap _links;
		void _add_and_remove_links(const LinkMap& add, const LinkMap& remove);
		void _replace_links(const LinkMap& links);
};

const LinkSet EMPTY_LINK_SET;
//const std::set<std::string> EMPTY_LINK_SET;

//typedef boost::function<void (const std::string&, bool, IUEventType, const std::string&)> IUEventHandlerFunction;
typedef boost::function<void (boost::shared_ptr<IUInterface>, IUEventType, bool)> IUEventHandlerFunction;

class IUEventHandler {
	protected:
		IUEventHandlerFunction _function;
		IUEventType _event_mask;
		bool _for_all_categories;
		std::set<std::string> _categories;
	protected:
		inline bool _condition_met(IUEventType event_type, const std::string& category)
		{
			return ((_event_mask&event_type)!=0) && (_for_all_categories || (_categories.count(category)>0));
		}
	public:
		IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category);
		IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		//void call(Buffer* buffer, const std::string& uid, bool local, IUEventType event_type, const std::string& category);
		void call(Buffer* buffer, boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	typedef boost::shared_ptr<IUEventHandler> ptr;
};

class Buffer { //: public boost::enable_shared_from_this<Buffer> {//{{{
	friend class IU;
	friend class RemotePushIU;
	protected:
		std::string _uuid;
		std::string _basename;
		std::string _unique_name;
		std::string _id_prefix;
		std::vector<IUEventHandler::ptr> _event_handlers;
	protected:
		_IPAACA_ABSTRACT_ virtual void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef") = 0;
		_IPAACA_ABSTRACT_ virtual void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") = 0;
		_IPAACA_ABSTRACT_ virtual void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
		void _allocate_unique_name(const std::string& basename, const std::string& function);
		inline Buffer(const std::string& basename, const std::string& function) {
			_allocate_unique_name(basename, function);
		}
	public:
		virtual inline ~Buffer() { }
		inline const std::string& unique_name() { return _unique_name; }
		void register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		void register_handler(IUEventHandlerFunction function, IUEventType event_mask = IU_ALL_EVENTS, const std::string& category="");
		//_IPAACA_ABSTRACT_ virtual void add(boost::shared_ptr<IUInterface> iu) = 0;
		_IPAACA_ABSTRACT_ virtual boost::shared_ptr<IUInterface> get(const std::string& iu_uid) = 0;
};
//}}}

class OutputBuffer: public Buffer { //, public boost::enable_shared_from_this<OutputBuffer>  {//{{{
	friend class IU;
	friend class RemotePushIU;
	protected:
		std::map<std::string, Informer<AnyType>::Ptr> _informer_store;
		IUStore _iu_store;
		Lock _iu_id_counter_lock;
	protected:
		// informing functions
		void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef");
		void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef");
		void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name);
		// remote access functions
		// _remote_update_links(IULinkUpdate)
		// _remote_update_payload(IUPayloadUpdate)
		// _remote_commit(protobuf::IUCommission)
	protected:
		void _publish_iu(boost::shared_ptr<IU> iu);
		void _retract_iu(boost::shared_ptr<IU> iu);
		Informer<AnyType>::Ptr _get_informer(const std::string& category);
	protected:
		OutputBuffer(const std::string& basename);
	public:
		static boost::shared_ptr<OutputBuffer> create(const std::string& basename);
		~OutputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		void add(boost::shared_ptr<IU> iu);
		boost::shared_ptr<IU> remove(const std::string& iu_uid);
		boost::shared_ptr<IU> remove(boost::shared_ptr<IU> iu);
		boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
	typedef boost::shared_ptr<OutputBuffer> ptr;
};
//}}}

class InputBuffer: public Buffer { //, public boost::enable_shared_from_this<InputBuffer>  {//{{{
	friend class IU;
	friend class RemotePushIU;
	protected:
		std::map<std::string, ListenerPtr> _listener_store;
		RemotePushIUStore _iu_store;  // TODO genericize
	protected:
		inline void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef")
		{
			IPAACA_INFO("(ERROR) InputBuffer::_send_iu_link_update() should never be invoked")
		}
		inline void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef")
		{
			IPAACA_INFO("(ERROR) InputBuffer::_send_iu_payload_update() should never be invoked")
		}
		inline void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
		{
			IPAACA_INFO("(ERROR) InputBuffer::_send_iu_commission() should never be invoked")
		}
	protected:
		RemoteServerPtr _get_remote_server(boost::shared_ptr<IU> iu);
		ListenerPtr _create_category_listener_if_needed(const std::string& category);
		void _handle_iu_events(EventPtr event);
		void call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	protected:
		InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests);
		InputBuffer(const std::string& basename, const std::string& category_interest1);
		InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);
	public:
		static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::vector<std::string>& category_interests);
		static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1);
		static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);
		~InputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
		//inline void add(boost::shared_ptr<IU> iu)
		//{
		//	IPAACA_IMPLEMENT_ME
		//}
	typedef boost::shared_ptr<InputBuffer> ptr;
};
//}}}

class IUConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IUConverter();
		std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}

class IUPayloadUpdate {//{{{
	public:
		std::string uid;
		revision_t revision;
		std::string writer_name;
		bool is_delta;
		std::map<std::string, std::string> new_items;
		std::vector<std::string> keys_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj);
	typedef boost::shared_ptr<IUPayloadUpdate> ptr;
};//}}}
class IUPayloadUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IUPayloadUpdateConverter();
		std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}

class IULinkUpdate {//{{{
	public:
		std::string uid;
		revision_t revision;
		std::string writer_name;
		bool is_delta;
		std::map<std::string, std::set<std::string> > new_links;
		std::map<std::string, std::set<std::string> > links_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj);
	typedef boost::shared_ptr<IULinkUpdate> ptr;
};//}}}
class IULinkUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IULinkUpdateConverter();
		std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}

void initialize_ipaaca_rsb();

class PayloadEntryProxy//{{{
{
	protected:
		Payload* _payload;
		std::string _key;
	public:
		PayloadEntryProxy(Payload* payload, const std::string& key);
		PayloadEntryProxy& operator=(const std::string& value);
		operator std::string();
		operator long();
		operator double();
		inline std::string to_str() { return operator std::string(); }
		inline long to_int() { return operator long(); }
		inline double to_float() { return operator double(); }
};//}}}

class Payload//{{{
{
	friend std::ostream& operator<<(std::ostream& os, const Payload& obj);
	friend class IUInterface;
	friend class IU;
	friend class RemotePushIU;
	friend class IUConverter;
	protected:
		std::string _owner_name;
		std::map<std::string, std::string> _store;
		boost::shared_ptr<IUInterface> _iu;
	protected:
		void initialize(boost::shared_ptr<IUInterface> iu);
		inline void _set_owner_name(const std::string& name) { _owner_name = name; }
		void _remotely_enforced_wipe();
		void _remotely_enforced_delitem(const std::string& k);
		void _remotely_enforced_setitem(const std::string& k, const std::string& v);
	public:
		inline const std::string& owner_name() { return _owner_name; }
		// access
		PayloadEntryProxy operator[](const std::string& key);
		void set(const std::string& k, const std::string& v);
		void remove(const std::string& k);
		std::string get(const std::string& k);
	typedef boost::shared_ptr<Payload> ref;
};//}}}

class IUInterface {//{{{
	friend class IUConverter;
	friend std::ostream& operator<<(std::ostream& os, const IUInterface& obj);
	protected:
		IUInterface();
	public:
		inline virtual ~IUInterface() { }
	protected:
		std::string _uid;
		revision_t _revision;
		std::string _category;
		std::string _payload_type; // default is "MAP"
		std::string _owner_name;
		bool _committed;
		IUAccessMode _access_mode;
		bool _read_only;
		//boost::shared_ptr<Buffer> _buffer;
		Buffer* _buffer;
		SmartLinkMap _links;
	protected:
		friend class Payload;
		// Internal functions that perform the update logic,
		//  e.g. sending a notification across the network
		_IPAACA_ABSTRACT_ virtual void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name) = 0;
		_IPAACA_ABSTRACT_ virtual void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name) = 0;
		//void _set_buffer(boost::shared_ptr<Buffer> buffer);
		void _associate_with_buffer(Buffer* buffer);
		void _set_buffer(Buffer* buffer);
		void _set_uid(const std::string& uid);
		void _set_owner_name(const std::string& owner_name);
	protected:
		// internal functions that do not emit update events
		inline void _add_and_remove_links(const LinkMap& add, const LinkMap& remove) { _links._add_and_remove_links(add, remove); }
		inline void _replace_links(const LinkMap& links) { _links._replace_links(links); }
	public:
		inline bool is_published() { return (_buffer != 0); }
		inline const std::string& uid() const { return _uid; }
		inline revision_t revision() const { return _revision; }
		inline const std::string& category() const { return _category; }
		inline const std::string& payload_type() const { return _payload_type; }
		inline const std::string& owner_name() const { return _owner_name; }
		inline bool committed() const { return _committed; }
		inline IUAccessMode access_mode() const { return _access_mode; }
		inline bool read_only() const { return _read_only; }
		//inline boost::shared_ptr<Buffer> buffer() { return _buffer; }
		inline Buffer* buffer() const { return _buffer; }
		inline const LinkSet& get_links(std::string type) { return _links.get_links(type); }
		inline const LinkMap& get_all_links() { return _links.get_all_links(); }
		// Payload
		_IPAACA_ABSTRACT_ virtual Payload& payload() = 0;
		_IPAACA_ABSTRACT_ virtual const Payload& const_payload() const = 0;
		// setters
		_IPAACA_ABSTRACT_ virtual void commit() = 0;
		// functions to modify and update links:
		void add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		void remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		void modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name = "");
		void set_links(const LinkMap& links, const std::string& writer_name = "");
		//    (with cpp specific convenience functions:)
		void add_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
		void remove_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
	typedef boost::shared_ptr<IUInterface> ptr;
};//}}}

class IU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	public:
		Payload _payload;
	protected:
		Lock _revision_lock;
	protected:
		inline void _increase_revision_number() { _revision++; }
		IU(const std::string& category, IUAccessMode access_mode=IU_ACCESS_PUSH, bool read_only=false, const std::string& payload_type="MAP" );
	public:
		inline ~IU() {
			IPAACA_IMPLEMENT_ME
		}
		static boost::shared_ptr<IU> create(const std::string& category, IUAccessMode access_mode=IU_ACCESS_PUSH, bool read_only=false, const std::string& payload_type="MAP" );
		inline Payload& payload() { return _payload; }
		inline const Payload& const_payload() const { return _payload; }
		void commit();
	protected:
		void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");
		void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		void _internal_commit(const std::string& writer_name = "");
	public:
	typedef boost::shared_ptr<IU> ref;
};//}}}

class RemotePushIU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	public:
		Payload _payload;
	protected:
		RemotePushIU();
		static boost::shared_ptr<RemotePushIU> create();
	public:
		inline ~RemotePushIU() {
			IPAACA_IMPLEMENT_ME
		}
		inline Payload& payload() { return _payload; }
		inline const Payload& const_payload() const { return _payload; }
		void commit();
	protected:
		void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "");
		void _modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "");
	protected:
		void _apply_update(IUPayloadUpdate::ptr update);
		void _apply_link_update(IULinkUpdate::ptr update);
		void _apply_commission();
	typedef boost::shared_ptr<RemotePushIU> ref;
};//}}}

class Exception: public std::exception//{{{
{
	protected:
		std::string _description;
		inline Exception(const std::string& description=""): _description(description) { }
	public:
		inline ~Exception() throw() { }
		const char* what() const throw() {
			return _description.c_str();
		}
};//}}}
class IUPublishedError: public Exception//{{{
{
	public:
		inline ~IUPublishedError() throw() { }
		inline IUPublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUPublishedError";
		}
};//}}}
class IUCommittedError: public Exception//{{{
{
	public:
		inline ~IUCommittedError() throw() { }
		inline IUCommittedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUCommittedError";
		}
};//}}}
class IUAlreadyInABufferError: public Exception//{{{
{
	public:
		inline ~IUAlreadyInABufferError() throw() { }
		inline IUAlreadyInABufferError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyInABufferError";
		}
};//}}}
class IUAlreadyHasAnUIDError: public Exception//{{{
{
	public:
		inline ~IUAlreadyHasAnUIDError() throw() { }
		inline IUAlreadyHasAnUIDError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnUIDError";
		}
};//}}}
class IUAlreadyHasAnOwnerNameError: public Exception//{{{
{
	public:
		inline ~IUAlreadyHasAnOwnerNameError() throw() { }
		inline IUAlreadyHasAnOwnerNameError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnOwnerNameError";
		}
};//}}}
class NotImplementedError: public Exception//{{{
{
	public:
		inline ~NotImplementedError() throw() { }
		inline NotImplementedError() { //boost::shared_ptr<IU> iu) {
			_description = "NotImplementedError";
		}
};//}}}

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

} // of namespace ipaaca

#endif


