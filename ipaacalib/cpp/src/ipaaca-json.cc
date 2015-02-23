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

#include <ipaaca/ipaaca.h>
#include <ipaaca/ipaaca-json.h>

#include <iomanip>

using namespace rapidjson;
using namespace std;

int fakeiu_main(int argc, char** argv)
{
	//if (argc<2) {
	//	std::cout << "Please provide json content as the first argument." << std::endl;
	//	return 0;
	//}
	//
	ipaaca::CommandLineParser::ptr parser = ipaaca::CommandLineParser::create();
	ipaaca::CommandLineOptions::ptr options = parser->parse(argc, argv);

	std::string json_source("[\"old\",2,3,4]");
	ipaaca::PayloadDocumentEntry::ptr entry = ipaaca::PayloadDocumentEntry::from_json_string_representation(json_source);
	
	ipaaca::FakeIU::ptr iu = ipaaca::FakeIU::create();
	iu->add_fake_payload_item("a", entry);
	iu->payload()["b"] = "anotherValue";
	iu->payload()["c"] = "yetAnotherValue";

	auto a = iu->payload()["a"];
	//auto a0 = a[0];
	std::cout << "entry as string:       " << (std::string) a << std::endl;
	std::cout << "entry as long:         " << (long) a << std::endl;
	std::cout << "entry as double:       " << (double) a << std::endl;
	std::cout << "entry as bool:         " << ((bool) a?"true":"false") << std::endl;
	// std::vector
	std::cout << "entry as vector<string>: ";
	try {
		std::vector<std::string> v = a;
		std::for_each(v.begin(), v.end(), [](std::string& s) {
				std::cout << s << " ";
		});
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	std::cout << "entry as vector<long>:   ";
	try {
		std::vector<long> v = a;
		std::for_each(v.begin(), v.end(), [](long& s) {
				std::cout << s << " ";
		});
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	std::cout << "entry as vector<bool>:   ";
	try {
		std::vector<bool> v = a;
		std::for_each(v.begin(), v.end(), [](bool s) {
				std::cout << (s?"true":"false") << " ";
		});
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	// std::map
	std::cout << "entry as map<string, string>: ";
	try {
		std::map<std::string, std::string> m = a;
		for (auto it = m.begin(); it != m.end(); ++it) {
			std::cout << it->first << ":" << it->second << "  ";
		}
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	std::cout << "entry as map<string, long>: ";
	try {
		std::map<std::string, long> m = a;
		for (auto it = m.begin(); it != m.end(); ++it) {
			std::cout << it->first << ":" << it->second << "  ";
		}
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	std::cout << "entry as map<string, double>: ";
	try {
		std::map<std::string, double> m = a;
		for (auto it = m.begin(); it != m.end(); ++it) {
			std::cout << it->first << ":" << it->second << "  ";
		}
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	std::cout << "entry as map<string, bool>: ";
	try {
		std::map<std::string, bool> m = a;
		for (auto it = m.begin(); it != m.end(); ++it) {
			std::cout << it->first << ":" << (it->second?"true":"false") << "  ";
		}
		std::cout << std::endl;
	} catch (...) {
		std::cout << "(n/a)" << std::endl;
	}
	
	std::cout << "Setting value [0] in the object:" << std::endl;
	try {
		iu->payload()["a"][0] = "CHANGED_BY_USER";
	} catch (ipaaca::PayloadAddressingError& e) {
		std::cout << "  Error - the provided object was not a suitable array" << std::endl;
	}
	//iu->payload()["a"]["A"] = "set by pep::op=";
	
	
	std::cout << "Appending two words to key 'b' the currently wrong way:" << std::endl;
	auto proxy = iu->payload()["b"];
	proxy = (std::string) proxy + " WORD1";
	proxy = (std::string) proxy + " WORD2";
	
	std::cout << "Appending two words to key 'c' the compatible way:" << std::endl;
	iu->payload()["c"] = (std::string) iu->payload()["c"] + " WORD1";
	iu->payload()["c"] = (std::string) iu->payload()["c"] + " WORD2";
	
	std::cout << "Printing final payload using PayloadIterator:" << std::endl;
	for (auto it = iu->payload().begin(); it != iu->payload().end(); ++it) {
		std::cout << "  " << std::left << std::setw(15) << ((*it).first+": ") << (*it).second << std::endl;
	}
	
	std::cout << "Final payload (cast to map, printed as strings):" << std::endl;
	std::map<std::string, std::string> pl_flat = iu->payload();
	for (auto& kv: pl_flat) {
		std::cout << "  " << std::left << std::setw(15) << (kv.first+": ") << kv.second << std::endl;
	}
	/*{
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		entry->document.Accept(writer);
		std::string docstring = buffer.GetString();
		std::cout << "Final document:  " << docstring << std::endl;
	}*/
	
	// Done
	return 0;
}


int legacy_iu_main(int argc, char** argv)
{
	// produce and fill a new and a legacy IU with identical contents
	
	ipaaca::OutputBuffer::ptr ob = ipaaca::OutputBuffer::create("jsonTestSenderLegacy");
	ob->register_handler([](ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local) {
		std::cout << "Received remote update, new payload: " << iu->payload() << std::endl;
	});
	std::cout << "--- Create IUs with category jsonTest" << std::endl;
	ipaaca::IU::ptr iu1 = ipaaca::IU::create("jsonTest", "JSON");
	ipaaca::IU::ptr iu2 = ipaaca::IU::create("jsonTest", "STR");
	std::map<std::string, long> newmap = { {"fifty", 50}, {"ninety-nine", 99} };
	std::cout << "--- Set map" << std::endl;
	iu1->payload()["map"] = newmap;
	iu1->payload()["array"] = std::vector<std::string>{"aaa", "bbb", "ccc"};
	iu2->payload()["map"] = newmap;
	iu2->payload()["array"] = std::vector<std::string>{"aaa", "bbb", "ccc"};
	std::cout << "--- Publishing IUs with this payload:" << std::endl;
	std::cout << iu1->payload() << std::endl;
	ob->add(iu1);
	ob->add(iu2);
	std::cout << "--- Waiting for changes for 5s " << std::endl;
	sleep(5);
	return 0;
}

int iu_main(int argc, char** argv)
{
	ipaaca::InputBuffer::ptr ib = ipaaca::InputBuffer::create("jsonTestReceiver", "jsonTest");
	ib->register_handler([](ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local) {
		if (event_type==IU_ADDED) {
			std::cout << "Received a new IU, payload: " << iu->payload() << std::endl;
			std::cout << "Will write something." << std::endl;
			//iu->commit();
			try {
				iu->payload()["list"][0] = "Overridden from C++";
			} catch (ipaaca::PayloadAddressingError& e) {
				iu->payload()["newKey"] = std::vector<long>{2,4,6,8};
				std::cout << "  (item ['list'][0] could not be addressed, wrote new key)" << std::endl;
			}
		}
	});
	std::cout << "--- Waiting for IUs for 10s " << std::endl;
	sleep(10);
	return 0;
	
	ipaaca::OutputBuffer::ptr ob = ipaaca::OutputBuffer::create("jsonTestSender");
	ob->register_handler([](ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local) {
		std::cout << "Received remote update, new payload: " << iu->payload() << std::endl;
	});
	std::cout << "--- Create IU with category jsonTest" << std::endl;
	ipaaca::IU::ptr iu = ipaaca::IU::create("jsonTest");
	std::map<std::string, long> newmap = { {"fifty", 50}, {"ninety-nine", 99} };
	std::cout << "--- Set map" << std::endl;
	iu->payload()["map"] = newmap;
	std::cout << "--- Publishing IU with this payload:" << std::endl;
	std::cout << iu->payload() << std::endl;
	ob->add(iu);
	std::cout << "--- Waiting for changes for 5s before next write" << std::endl;
	sleep(5);
	std::cout << "--- Contents of map after 5s" << std::endl;
	std::cout << iu->payload()["map"] << std::endl;
	//
	std::cout << "--- Creating a list" << std::endl;
	iu->payload()["list"] = std::vector<long>{1, 0} ;
	std::cout << "--- Waiting for changes for 5s " << std::endl;
	sleep(5);
	std::cout << "--- Final map " << std::endl;
	std::cout << iu->payload()["map"] << std::endl;
	std::cout << "--- Final list " << std::endl;
	std::cout << iu->payload()["list"] << std::endl;
	std::cout << "--- Terminating " << std::endl;
	return 0;
}

int main(int argc, char** argv)
{
	return legacy_iu_main(argc, argv);
	//return fakeiu_main(argc, argv);
	//return iu_main(argc, argv);
}
