#include <ipaaca.h>
#include <typeinfo>

//#include <rsc/logging/Logger.h>
//#include <rsc/logging/LoggerFactory.h>
// //rsc::logging::LoggerFactory::getInstance().reconfigure(rsc::logging::Logger::LEVEL_ALL);

//
//   TESTS
//

using namespace ipaaca;

#ifdef MAKE_RECEIVER
void my_first_iu_handler(IUInterface::ptr iu, IUEventType type, bool local)
{
	std::cout << "[32m" << iu_event_type_to_str(type) << " " << (local?"(of local IU)":"(of remote IU)") << "[m" << std::endl;
	if (type == IU_LINKSUPDATED) {
		std::cout << "  setting something in the remote payload" << std::endl;
		iu->payload()["new_field"] = "remotely_set";
	}
}
int main() {
	try{
		//initialize_ipaaca_rsb();
		
		InputBuffer::ptr ib = InputBuffer::create("Tester", "testcategory");
		ib->register_handler(my_first_iu_handler);
		
		while (true) {
			sleep(1);
		}
		
	} catch (ipaaca::Exception& e) {
		std::cout << "== IPAACA EXCEPTION == " << e.what() << std::endl;
	}
}
#else
#ifdef MAKE_SENDER
void iu_handler_for_remote_changes(IUInterface::ptr iu, IUEventType type, bool local)
{
	std::cout << "[32m" << iu_event_type_to_str(type) << " " << (local?"(of local IU)":"(of remote IU)") << "[m" << std::endl;
}
int main() {
	try{
		//initialize_ipaaca_rsb();
		
		
		OutputBuffer::ptr ob = OutputBuffer::create("Tester");
		ob->register_handler(iu_handler_for_remote_changes);
		//std::cout << "Buffer: " << ob->unique_name() << std::endl;
		
		IU::ptr iu = IU::create("testcategory");
		ob->add(iu);
		
		std::cout << "_payload.get(\"TEST\") = \"" << iu->_payload.get("TEST") << "\"" << std::endl;
		std::cout << "_payload[\"TEST\"] = \"" << (std::string) iu->_payload["TEST"] << "\"" << std::endl;
		iu->_payload["TEST"] = "123.5-WAS-SET";
		std::cout << "_payload[\"TEST\"] = \"" << (std::string) iu->_payload["TEST"] << "\"" << std::endl;
		
		std::string s = "The string \"" + iu->_payload["TEST"].to_str() + "\" is the new value.";
		std::cout << "Concatenation test: " << s << std::endl;
		
		iu->add_link("grin", "DUMMY_IU_UID_1234efef1234");
		
		std::cout << "Interpreted as  long  value: " << iu->_payload["TEST"].to_int() << std::endl;
		std::cout << "Interpreted as double value: " << iu->_payload["TEST"].to_float() << std::endl;
		
		std::cout << "Committing and quitting in 1 sec" << std::endl;
		sleep(1);
		iu->commit();
	} catch (ipaaca::Exception& e) {
		std::cout << "== IPAACA EXCEPTION == " << e.what() << std::endl;
	}
}

#endif
#endif

