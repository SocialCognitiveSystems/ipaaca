#ifndef __IPAACA_H__
#define __IPAACA_H_

#ifdef IPAACA_DEBUG_MESSAGES
#define IPAACA_INFO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- " << i << std::endl;
#define IPAACA_IMPLEMENT_ME(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- IMPLEMENT ME" << std::endl;
#define IPAACA_TODO(i) std::cout << __FILE__ << ":" << __LINE__ << ": " << __func__ << "() -- TODO: " << i << std::endl;
#else
#define IPAACA_INFO(i) ;
#define IPAACA_IMPLEMENT_ME(i) ;
#define IPAACA_TODO(i) ;
#endif

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

//using namespace boost;
using namespace rsb;
using namespace rsb::converter;

namespace ipaaca {

enum IUEventType {
	ADDED,
	COMMITTED,
	DELETED,
	RETRACTED,
	UPDATED,
	LINKSUPDATED
};

enum IUAccessMode {
	PUSH,
	REMOTE,
	MESSAGE
};

class Payload;
class IUInterface;
class IU;
class RemotePushIU;
// class IULinkUpdate
// class IULinkUpdateConverter;
class IUStore;
class FrozenIUStore;
class IUEventHandler;

class Buffer {
};

class InputBuffer: public Buffer {
};

class OutputBuffer: public Buffer {
};

/*
class IUEventFunctionHandler: public rsb::EventFunctionHandler {
	protected:
		Buffer* _buffer;
	public:
		inline IUEventFunctionHandler(Buffer* buffer, const EventFunction& function, const std::string& method="")
			: EventFunctionHandler(function, method), _buffer(buffer) { }
};
*/

class IUPayloadUpdate {
	public:
		std::string uid;
		uint32_t revision;
		std::string writer_name;
		bool is_delta;
		std::map<std::string, std::string> new_items;
		std::vector<std::string> keys_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj);
};
class IUPayloadUpdateConverter: public rsb::converter::Converter<std::string> {
	public:
		IUPayloadUpdateConverter();
		std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};

class IULinkUpdate {
	public:
		std::string uid;
		uint32_t revision;
		std::string writer_name;
		bool is_delta;
		std::map<std::string, std::set<std::string> > new_links;
		std::map<std::string, std::set<std::string> > links_to_remove;
	friend std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj);
};
class IULinkUpdateConverter: public rsb::converter::Converter<std::string> {
	public:
		IULinkUpdateConverter();
		std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};


void initialize_ipaaca_rsb();

class PayloadEntryProxy
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
};

class Payload
{
	protected:
		std::map<std::string, std::string> _store;
		boost::shared_ptr<IUInterface> _iu;
	protected:
		friend class IU;
		friend class RemotePushIU;
		Payload();
		void initialize(boost::shared_ptr<IUInterface> iu);
	public:
		PayloadEntryProxy operator[](const std::string& key);
		void set(const std::string& k, const std::string& v);
		void remove(const std::string& k);
		std::string get(const std::string& k);
};

class IUInterface {
	public:
		inline virtual ~IUInterface() { }
};

class IU: public IUInterface {
	public:
		Payload payload;
	protected:
		inline IU() { }
	public:
		inline ~IU() { }
		static boost::shared_ptr<IU> create();
	typedef boost::shared_ptr<IU> ref;
};

class RemotePushIU: public IUInterface {
	public:
		inline ~RemotePushIU() { }
};

class IUPublishedError: public std::exception
{
	protected:
		std::string _description;
	public:
		inline ~IUPublishedError() throw() { }
		inline IUPublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUPublishedError";
		}
		const char* what() const throw() {
			return _description.c_str();
		}
};


} // of namespace ipaaca

#endif


