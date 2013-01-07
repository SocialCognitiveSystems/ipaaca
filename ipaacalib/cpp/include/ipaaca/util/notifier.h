#ifndef __IPAACA_UTIL_NOTIFIER_H__
#define __IPAACA_UTIL_NOTIFIER_H__

#include <ipaaca/ipaaca.h>

#define _IPAACA_COMP_NOTIF_CATEGORY "componentNotify"

#define _IPAACA_COMP_NOTIF_NAME      "name"
#define _IPAACA_COMP_NOTIF_FUNCTION  "function"
#define _IPAACA_COMP_NOTIF_STATE     "state"
#define _IPAACA_COMP_NOTIF_SEND_CATS "send_categories"
#define _IPAACA_COMP_NOTIF_RECV_CATS "receive_categories"

#define _IPAACA_COMP_NOTIF_STATE_NEW  "new"
#define _IPAACA_COMP_NOTIF_STATE_OLD  "old"
#define _IPAACA_COMP_NOTIF_STATE_DOWN "down"

namespace ipaaca {
namespace util {

class ComponentNotifier {
	protected:
		ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories);
		ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr out_buf, ipaaca::InputBuffer::ptr in_buf);
	public:
		static boost::shared_ptr<ComponentNotifier> create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories);
		static boost::shared_ptr<ComponentNotifier> create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr out_buf, ipaaca::InputBuffer::ptr in_buf);
	public:
		~ComponentNotifier();
	protected:
		void submit_notify(const std::string& current_state);
		void handle_iu_event(ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local);
	public:
		void add_notification_handler(ipaaca::IUEventHandlerFunction function);
		void initialize();
	protected:
		ipaaca::OutputBuffer::ptr out_buf;
		ipaaca::InputBuffer::ptr in_buf;
		ipaaca::Lock lock;
		bool initialized;
		std::vector<ipaaca::IUEventHandlerFunction> _handlers;
	protected:
		std::string name;
		std::string function;
		std::string state;
		std::string send_categories;
		std::string recv_categories;
	public:
	typedef boost::shared_ptr<ComponentNotifier> ptr;
};


}} // of namespace ipaaca::util

#endif

