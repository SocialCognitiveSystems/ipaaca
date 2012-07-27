//
// Ipaaca 2 (ipaaca-rsb)
//
// *****************************
// ***                       ***
// ***   C++ usage example   ***
// ***                       ***
// *****************************
//
// Example class highlighting the use of the C++ interface
// to ipaaca2. This class uses a setup similar to an ipaaca1
// 'Component' (the concept does not exist anymore in ipaaca2).
//

#include <ipaaca.h>
#include <typeinfo>

using namespace ipaaca;

/// Test component for ipaaca2
///  The interface is much more flexible than in ipaaca1.
///  This example class provides an interface similar to ipaaca1.
class LegacyComponent {
	protected:
		/// Make a buffer pair, as in ipaaca1.
		///  NOTE1: This is no longer a restriction. You can have
		///  multiple buffers, no input buffer, etc.
		///  NOTE2: Most objects are referred to using smart pointers
		///    using the type name <className>::ptr - don't use '*'
		OutputBuffer::ptr _out_buf;
		InputBuffer::ptr  _in_buf;
	public:
		/// Constructor to set up the component
		LegacyComponent();
		/// IU event handler function. Can be a member function,
		///   a static function, anything. Use boost::bind on registration.
		///   NOTE1: you can register any number of handlers on any Buffer.
		///   NOTE2: this example function has the appropriate signature.
		void handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local);
		/// example publishing function to produce a 'grounded' IU
		void publish_reply_iu(const std::string& text, const std::string& received_iu_uid);
		void publish_hello_world();
};

LegacyComponent::LegacyComponent() {
	/// First create the buffer pair
	///  Create an output buffer
	_out_buf = OutputBuffer::create("MyOutputBuffer");
	///  Create an input buffer with category interest
	///    NOTE: You can pass up to four categories as strings
	//       to the constructor, or use an std::vector<std::string>
	_in_buf = InputBuffer::create("MyInputBuffer", "myCategoryInterest");
	
	/// Now register the IU handler on both buffers.
	///   NOTE1: we could register separate handlers instead.
	///   NOTE2: boost::bind enables use of simple closures:
	///     You could specify constants for the handler function, as long as the
	///      remaining open arguments form the correct signature for IU handlers.
	///      ** If you simply want to use a class member function, use it as below **
	///    NOTE3: the Buffers are 'live' immediately on creation. As soon as
	///      you connect a handler, it can be triggered (no backend connect etc.).
	///      If this is not what you want, you should set a flag when you are
	///      ready to actually start, and have the handlers honor that flag.
	_out_buf->register_handler(boost::bind(&LegacyComponent::handle_iu_event, this, _1, _2, _3));
	_in_buf->register_handler(boost::bind(&LegacyComponent::handle_iu_event, this, _1, _2, _3));
}

void LegacyComponent::handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	if (local) {
		std::cout << "[Received update of self-owned IU]" << std::endl;
		// could do processing here to cope with remote change of own IU
	
	} else {
		// event on a remote IU
		if (event_type == IU_ADDED) {
			std::cout << "[Received new IU!]" << std::endl;
			
			/// new Payload class enables dynamic typing to some degree (numeric default 0)
			std::string description = iu->payload()["description"];
			double fraction = iu->payload()["fraction"];
			/// let's also get the grounded-in links
			std::set<std::string> grin_links = iu->get_links("GRIN");
			
			std::cout << "[ Current description: " << description << "]" << std::endl;
			
			/// let's also react by emitting an IU ourselves (function below)
			publish_reply_iu("important-result", iu->uid());
		
		} else if (event_type == IU_UPDATED) {
			std::cout << "[Received IU payload update for IU " << iu->uid() << "]" << std::endl;
			std::string description = iu->payload()["description"];
			std::cout << "[ Current description: " << description << "]" << std::endl;
		
		} else if (event_type == IU_LINKSUPDATED) {
			std::cout << "[IU links updated.]" << std::endl;
			
		} else if (event_type == IU_COMMITTED) {
			std::cout << "[IU " << iu->uid() << " has been committed to.]" << std::endl;
		
		} else if (event_type == IU_RETRACTED) {
			std::cout << "[IU " << iu->uid() << " has been retracted.]" << std::endl;
		
		} else if (event_type == IU_DELETED) {
			std::cout << "[IU " << iu->uid() << " has been deleted.]" << std::endl;
		
		} else {
			// Possible to stringify the type:
			std::cout << "[(IU event " << iu_event_type_to_str(event_type) << " " << iu->uid() << ")]" << std::endl;
		}
	}
}

void LegacyComponent::publish_reply_iu(const std::string& text, const std::string& received_iu_uid) {
	/// create a new IU
	IU::ptr iu = IU::create( "myResultCategory" );
	/// Add something to the payload
	iu->payload()["description"] = "SomeResult";
	/// Now add a grounded-in link pointing to the IU received before
	///   There are no limitations to the link group names.
	///   "GRIN" is a convention for the "grounded-in" function of the GAM.
	iu->add_link("GRIN", received_iu_uid);
	/// add to output buffer ( = "publish")
	_out_buf->add(iu);
}

void LegacyComponent::publish_hello_world() {
	IU::ptr iu = IU::create( "myCategoryInterest"); //helloWorld" );
	iu->payload()["description"] = "Hello world";
	_out_buf->add(iu);
}

int main() {
	std::cout << "Creating buffers..." << std::endl;
	LegacyComponent compo;
	sleep(1);
	std::cout << "Publishing an initial IU..." << std::endl;
	compo.publish_hello_world();
	std::cout << "*** Running main loop, press Ctrl-C to cancel... ***" << std::endl;
	/// NOTE: custom main loop no longer required.
	while (true) sleep(1);
}

