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
		
		std::cout << "Updating in 1 sec" << std::endl;
		sleep(1);
		
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

