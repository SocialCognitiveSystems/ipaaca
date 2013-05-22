#include <ipaaca/util/notifier.h>

namespace ipaaca {
namespace util {

ComponentNotifier::~ComponentNotifier() 
{
	if (initialized) {
		submit_notify(_IPAACA_COMP_NOTIF_STATE_DOWN);
	}
}

ComponentNotifier::ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories)
: initialized(false), name(componentName), function(componentFunction)
{
	send_categories = ipaaca::str_join(sendCategories, ",");
	recv_categories = ipaaca::str_join(recvCategories, ",");
	// create private in/out buffer pair since none was specified
	out_buf = ipaaca::OutputBuffer::create(componentName);
	in_buf = ipaaca::InputBuffer::create(componentName, _IPAACA_COMP_NOTIF_CATEGORY);
}

ComponentNotifier::ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr outBuf, ipaaca::InputBuffer::ptr inBuf)
: initialized(false), name(componentName), function(componentFunction), out_buf(outBuf), in_buf(inBuf)
{
	send_categories = ipaaca::str_join(sendCategories, ",");
	recv_categories = ipaaca::str_join(receiveCategories, ",");
}

ComponentNotifier::ptr ComponentNotifier::create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories)
{
	return ComponentNotifier::ptr(new ComponentNotifier(componentName, componentFunction, sendCategories, recvCategories));
}
ComponentNotifier::ptr ComponentNotifier::create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories, ipaaca::OutputBuffer::ptr outBuf, ipaaca::InputBuffer::ptr inBuf)
{
	return ComponentNotifier::ptr(new ComponentNotifier(componentName, componentFunction, sendCategories, recvCategories, outBuf, inBuf));
}

void ComponentNotifier::handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	//std::cout << "handle_iu_event: got an event" << std::endl;
	if ((event_type == IU_ADDED) || (event_type == IU_UPDATED) || (event_type == IU_MESSAGE)) {
		Locker locker(lock);
		std::string cName = iu->payload()[_IPAACA_COMP_NOTIF_NAME];
		std::string cState = iu->payload()[_IPAACA_COMP_NOTIF_STATE];
		if (cName != name) {
			//std::cout << " handle_iu_event: calling notification handlers" << std::endl;
			// call all registered notification handlers
			for (std::vector<IUEventHandlerFunction>::iterator it = _handlers.begin(); it != _handlers.end(); ++it) {
				(*it)(iu, event_type, local);
			}
			// send own info only if the remote component is a newly initialized one
			if (cState=="new") {
				//std::cout << " handle_iu_event: Submitting own notification to new remote end" << std::endl;
				submit_notify(_IPAACA_COMP_NOTIF_STATE_OLD);
			}
		}
	}
}

void ComponentNotifier::add_notification_handler(ipaaca::IUEventHandlerFunction function)
{
	Locker locker(lock);
	_handlers.push_back(function);
}

void ComponentNotifier::submit_notify(const std::string& current_state)
{
	ipaaca::Message::ptr iu = ipaaca::Message::create(_IPAACA_COMP_NOTIF_CATEGORY);
	iu->payload()[_IPAACA_COMP_NOTIF_NAME] = name;
	iu->payload()[_IPAACA_COMP_NOTIF_STATE] = current_state;
	iu->payload()[_IPAACA_COMP_NOTIF_FUNCTION] = function;
	iu->payload()[_IPAACA_COMP_NOTIF_SEND_CATS] = send_categories;
	iu->payload()[_IPAACA_COMP_NOTIF_RECV_CATS] = recv_categories;
	out_buf->add(iu);
	//LOG_IPAACA_CONSOLE( "Sending a ComponentNotify: " << name << " " << function << " " << current_state << " " << send_categories << " " << recv_categories )
}

void ComponentNotifier::initialize() {
	Locker locker(lock);
	if (!initialized) {
		initialized = true;
		in_buf->register_handler(boost::bind(&ComponentNotifier::handle_iu_event, this, _1, _2, _3));
		submit_notify(_IPAACA_COMP_NOTIF_STATE_NEW);
	}
}

}}

