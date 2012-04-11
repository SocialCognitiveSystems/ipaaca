#include <ipaaca.h>
#include <typeinfo>

//#include <rsc/logging/Logger.h>
//#include <rsc/logging/LoggerFactory.h>
// //rsc::logging::LoggerFactory::getInstance().reconfigure(rsc::logging::Logger::LEVEL_ALL);

#ifdef MAKE_RECEIVER
//boost::mutex mtx;
using namespace ipaaca;

class Testo {
	protected:
		std::string _name;
	public:
		inline Testo(const std::string& name="Testo"): _name(name) { }
		inline void handleIUEvent(EventPtr event)
		{
			std::cout << _name << " received a ";
			std::string type = event->getType();
			if (type == "ipaaca::IUPayloadUpdate") {
				std::cout << *boost::static_pointer_cast<IUPayloadUpdate>(event->getData()) << std::endl;
			} else if (type == "ipaaca::IULinkUpdate") {
				std::cout << *boost::static_pointer_cast<IULinkUpdate>(event->getData()) << std::endl;
			} else {
				std::cout << type << " (Unhandled type!)" << std::endl;
			}
		}
};

int main() {
	initialize_ipaaca_rsb();
	
	ListenerPtr listener = Factory::getInstance().createListener( Scope("/tutorial/converter"));
	
	Testo t("TESTO");
	HandlerPtr event_handler = HandlerPtr(new EventFunctionHandler(boost::bind(&Testo::handleIUEvent, boost::ref(t), _1)));
	listener->addHandler( event_handler );
	
	while(true) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
	return EXIT_SUCCESS;
}
#else
#ifdef MAKE_SENDER
using namespace ipaaca;
int main() {
	initialize_ipaaca_rsb();
	
	//Informer<ipaaca::IUPayloadUpdate>::Ptr pinformer = Factory::getInstance().createInformer<ipaaca::IUPayloadUpdate> ( Scope("/tutorial/converter"));
	//Informer<ipaaca::IULinkUpdate>::Ptr linformer = Factory::getInstance().createInformer<ipaaca::IULinkUpdate> ( Scope("/tutorial/converter"));
	
	Informer<AnyType>::Ptr informer = Factory::getInstance().createInformer<AnyType> ( Scope("/tutorial/converter"));

	IUPayloadUpdate* pup = new ipaaca::IUPayloadUpdate();
	Informer<ipaaca::IUPayloadUpdate>::DataPtr pdata(pup);
	pup->uid = "2000";
	pup->revision = 3;
	pup->writer_name = "Comp1_OB";
	pup->is_delta = true;
	pup->new_items["new_key"] = "new_value";
	pup->new_items["another_key"] = "some_info";
	pup->keys_to_remove.push_back("old_key");
	informer->publish(pdata);
	
	IULinkUpdate* lup = new ipaaca::IULinkUpdate();
	Informer<ipaaca::IULinkUpdate>::DataPtr ldata(lup);
	lup->uid = "2001";
	lup->revision = 4;
	lup->writer_name = "Comp2_IB";
	lup->is_delta = true;
	lup->new_links["SLL"].insert("2000");
	lup->new_links["grin"].insert("1002");
	lup->links_to_remove["grin"].insert("1001");
	informer->publish(ldata);
	
	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}
#else

//
//   TESTS
//

using namespace ipaaca;

int main() {
	try{
		initialize_ipaaca_rsb();
		
		
		OutputBuffer ob("TestOB");
		std::cout << "Buffer: " << ob.unique_name() << std::endl;
		
		IU::ref iu = IU::create("testcategory");
		ob.add(iu);
		
		std::cout << "_payload.get(\"TEST\") = \"" << iu->_payload.get("TEST") << "\"" << std::endl;
		std::cout << "_payload[\"TEST\"] = \"" << (std::string) iu->_payload["TEST"] << "\"" << std::endl;
		iu->_payload["TEST"] = "123.5-WAS-SET";
		std::cout << "_payload[\"TEST\"] = \"" << (std::string) iu->_payload["TEST"] << "\"" << std::endl;
		
		std::string s = "The string \"" + iu->_payload["TEST"].to_str() + "\" is the new value.";
		std::cout << "Concatenation test: " << s << std::endl;
	
		std::cout << "Interpreted as  long  value: " << iu->_payload["TEST"].to_int() << std::endl;
		std::cout << "Interpreted as double value: " << iu->_payload["TEST"].to_float() << std::endl;
		iu->commit();
	} catch (ipaaca::Exception& e) {
		std::cout << "== IPAACA EXCEPTION == " << e.what() << std::endl;
	}
}

#endif
#endif

