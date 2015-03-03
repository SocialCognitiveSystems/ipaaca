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

int iterators_main(int argc, char** argv)//{{{
{
	std::string json_source("[\n\
	\"old\",\n\
	[\n\
		\"str\",\n\
		null\n\
	],\n\
	3,\n\
	{\n\
		\"key1\": \"value1\",\n\
		\"key2\": \"value2\"\n\
	}\n\
]");
	
	std::cout << "Using this JSON document as initial payload entry 'a':" << std::endl << json_source << std::endl;
	ipaaca::PayloadDocumentEntry::ptr entry = ipaaca::PayloadDocumentEntry::from_json_string_representation(json_source);
	
	std::cout << std::endl << "Setting up payload by adding some additional values" << std::endl;
	ipaaca::FakeIU::ptr iu = ipaaca::FakeIU::create();
	iu->add_fake_payload_item("a", entry);
	iu->payload()["b"] = "simpleString";
	iu->payload()["bPrime"] = "simpleString";
	iu->payload()["c"] = "anotherSimpleString";
	iu->payload()["d"] = 100;
	iu->payload()["e"] = 3l;
	iu->payload()["f"] = "12.5000";
	
	std::cout << std::endl << "Iterate over payload" << std::endl;
	for (auto it = iu->payload().begin(); it != iu->payload().end(); ++it) {
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}
	std::cout << std::endl << "Iterate over payload, range-based" << std::endl;
	for (auto it: iu->payload()) {
		std::cout << "  " << it.first << " -> " << it.second << std::endl;
	}
	
	std::cout << std::endl << "Comparisons" << std::endl;
	bool eq;
	eq = iu->payload()["a"] == iu->payload()["b"];
	std::cout << "  a==b ? : " << (eq?"true":"false") << std::endl;
	eq = iu->payload()["b"] == iu->payload()["bPrime"];
	std::cout << "  b==bPrime ? : " << (eq?"true":"false") << std::endl;
	eq = iu->payload()["b"] == "simpleString";
	std::cout << "  b==\"simpleString\" ? : " << (eq?"true":"false") << std::endl;
	eq = iu->payload()["b"] == 100;
	std::cout << "  b==100 ? : " << (eq?"true":"false") << std::endl;
	eq = iu->payload()["d"] == 100;
	std::cout << "  d==100 ? : " << (eq?"true":"false") << std::endl;
	eq = iu->payload()["a"][2] == iu->payload()["e"];
	std::cout << "  a[2]==e ? : " << (eq?"true":"false") << std::endl;
	
	std::cout << std::endl << "Type checks" << std::endl;
	std::cout << "  a[3] is_null() ? : " << ((iu->payload()["a"][3].is_null())?"true":"false") << std::endl;
	std::cout << "  a[3] is_string() ? : " << ((iu->payload()["a"][3].is_string())?"true":"false") << std::endl;
	std::cout << "  a[3] is_number() ? : " << ((iu->payload()["a"][3].is_number())?"true":"false") << std::endl;
	std::cout << "  a[3] is_list() ? : " << ((iu->payload()["a"][3].is_list())?"true":"false") << std::endl;
	std::cout << "  a[3] is_map() ? : " << ((iu->payload()["a"][3].is_map())?"true":"false") << std::endl;
	std::cout << std::endl;
	std::cout << "  f is_null() ? : " << ((iu->payload()["f"].is_null())?"true":"false") << std::endl;
	std::cout << "  f is_string() ? : " << ((iu->payload()["f"].is_string())?"true":"false") << std::endl;
	std::cout << "  f is_number() ? : " << ((iu->payload()["f"].is_number())?"true":"false") << std::endl;
	std::cout << "  f is_list() ? : " << ((iu->payload()["f"].is_list())?"true":"false") << std::endl;
	std::cout << "  f is_map() ? : " << ((iu->payload()["f"].is_map())?"true":"false") << std::endl;
	
	std::cout << std::endl << "Inner iterators, map (printing values as strings)" << std::endl;
	try {
		auto inner = iu->payload()["a"][3];
		std::cout << "Map iteration over payload['a'][3], which equals " << inner << std::endl;
		std::cout << "Reported size is " << inner.size() << std::endl;
		for (auto kv: inner.as_map()) {
			std::cout << "  \"" << kv.first << "\" -> \"" << kv.second << "\"" << std::endl;
		}
	} catch (ipaaca::Exception& ex) {
		std::cout << "  Unexpected exception: " << ex.what() << std::endl;
	}
	try {
		auto inner = iu->payload()["a"][2];
		std::cout << "Map iteration over payload['a'][2], which equals " << inner << std::endl;
		std::cout << "Reported size is " << inner.size() << std::endl;
		for (auto kv: inner.as_map()) {
			std::cout << "  \"" << kv.first << "\" -> \"" << kv.second << "\"" << std::endl;
		}
	} catch (ipaaca::PayloadTypeConversionError& ex) {
		std::cout << "  Failed as expected with " << ex.what() << std::endl;
	} catch (ipaaca::Exception& ex) {
		std::cout << "  Unexpected exception: " << ex.what() << std::endl;
	}
	
	std::cout << std::endl << "Inner iterators, list (printing values as strings)" << std::endl;
	try {
		auto inner = iu->payload()["a"][1];
		std::cout << "List iteration over payload['a'][1], which equals " << inner << std::endl;
		std::cout << "Reported size is " << inner.size() << std::endl;
		for (auto proxy: inner.as_list()) {
			std::cout << "  \"" << proxy << "\"" << std::endl;
		}
	} catch (ipaaca::Exception& ex) {
		std::cout << "  Unexpected exception: " << ex.what() << std::endl;
	}
	try {
		auto inner = iu->payload()["a"][1][1];
		std::cout << "List iteration over payload['a'][1][1], which equals " << inner << std::endl;
		std::cout << "Reported size is " << inner.size() << std::endl;
		for (auto proxy: inner.as_list()) {
			std::cout << "  \"" << proxy << "\"" << std::endl;
		}
	} catch (ipaaca::PayloadTypeConversionError& ex) {
		std::cout << "  Failed as expected with " << ex.what() << std::endl;
	} catch (ipaaca::Exception& ex) {
		std::cout << "  Unexpected exception: " << ex.what() << std::endl;
	}
	
	return 0;
}
//}}}

int json_testbed_main(int argc, char** argv)//{{{
{
	std::string json_source("[\"old\",2,3,4]");
	ipaaca::PayloadDocumentEntry::ptr entry = ipaaca::PayloadDocumentEntry::from_json_string_representation(json_source);
	
	std::string newinner("{\"K\":\"V\"}");
	ipaaca::PayloadDocumentEntry::ptr entrynew = ipaaca::PayloadDocumentEntry::from_json_string_representation(newinner);
	
	ipaaca::FakeIU::ptr iu = ipaaca::FakeIU::create();
	iu->add_fake_payload_item("a", entry);
	iu->add_fake_payload_item("b", entrynew);
	iu->payload()["c"] = "simpleString";
	
	auto proxy = iu->payload()["a"][3];
	
	std::cout << "IU payload before: " << iu->payload() << std::endl;
	std::cout << "Entry before:      " << entry << std::endl;
	std::cout << "EntryNew before:   " << entrynew << std::endl;
	
	/*
	proxy.json_value->CopyFrom(entrynew->document, proxy.document_entry->document.GetAllocator());
	proxy.document_entry->update_json_source();
	*/
	proxy = iu->payload()["b"];
	
	std::cout << "Newly written part: " << iu->payload()["a"][3] << std::endl;
	iu->payload()["a"][3]["addkey"] = "addvalue";
	
	std::cout << "IU payload after: " << iu->payload() << std::endl;
	std::cout << "Entry after:      " << entry << std::endl;
	std::cout << "EntryNew after:   " << entrynew << std::endl;
	
	return 0;
}
//}}}

int fakeiu_main(int argc, char** argv)//{{{
{
	//if (argc<2) {
	//	std::cout << "Please provide json content as the first argument." << std::endl;
	//	return 0;
	//}
	//
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
	return 0;
}
//}}}

int legacy_iu_main(int argc, char** argv)//{{{
{
	// produce and fill a new and a legacy IU with identical contents
	
	ipaaca::OutputBuffer::ptr ob = ipaaca::OutputBuffer::create("jsonTestSenderLegacy");
	ob->register_handler([](ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local) {
		std::cout << "Received remote update, new payload: " << iu->payload() << std::endl;
	});
	std::cout << "--- Create IUs with category jsonTest" << std::endl;
	ipaaca::IU::ptr iu1 = ipaaca::IU::create("jsonTest");
	ipaaca::IU::ptr iu2 = ipaaca::IU::create("jsonTest", "STR"); // explicity request old payload
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
//}}}

int iu_main(int argc, char** argv)//{{{
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
//}}}

int main(int argc, char** argv)
{
	ipaaca::CommandLineParser::ptr parser = ipaaca::CommandLineParser::create();
	ipaaca::CommandLineOptions::ptr options = parser->parse(argc, argv);

	return iterators_main(argc, argv);
	//return json_testbed_main(argc, argv);
	//return legacy_iu_main(argc, argv);
	//return fakeiu_main(argc, argv);
	//return iu_main(argc, argv);
}
