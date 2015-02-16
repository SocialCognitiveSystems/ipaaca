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

int main(int argc, char** argv) {

	if (argc<2) {
		std::cout << "Please provide json content as the first argument." << std::endl;
		return 0;
	}
	std::string json_source(argv[1]);
	ipaaca::PayloadDocumentEntry::ptr entry = ipaaca::PayloadDocumentEntry::from_json_string_representation(json_source);
	
	ipaaca::FakeIU::ptr iu = ipaaca::FakeIU::create();
	iu->add_fake_payload_item("a", entry);

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
	
	//iu->payload()["b"] = "newEntry";
	
	//std::vector<long> vs = { 10, 20, 30, 40 };
	//std::map<std::string, double> vs = { {"A", 10}, {"B", 20}, {"C", 30}, {"D", 40} };
	//ipaaca::pack_into_json_value(entry->document, entry->document.GetAllocator(), vs);
	std::cout << "Setting a value deep in the object:" << std::endl;
	//iu->payload()["a"][(int)0] = "set by pep::op=";
	iu->payload()["a"]["A"] = "set by pep::op=";
	
	std::cout << "Final payload (printed as strings):" << std::endl;
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
