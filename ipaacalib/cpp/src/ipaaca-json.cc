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
	std::cout << "entry as list<string>: ";
	try {
	} catch {
		std::cout << "(Error)" << std::endl;
	}
	return 0;

	////////////////////////////////////////////////////////////////////////////
	// 1. Parse a JSON text string to a document.
	const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4], \"dict\":{\"s\":\"stringvalue\", \"arr\":[6, 7, \"test\"]} } ";
	printf("Original JSON:\n %s\n", json);
	
	ipaaca::PayloadDocumentStore ds;
	ds["document_test"] = std::make_shared<ipaaca::PayloadDocumentEntry>();
	Document& document = ds["document_test"]->document;

	//std::map<std::string, Document> documents;
	//Document _document; // Default template parameter uses UTF8 and MemoryPoolAllocator.
	//documents["document_test"] = std::move(_document);
	//Document& document = documents["document_test"];
	
	printf("Check whether document contains 'none' initially ...");
	assert(document.IsNull()); // initial state of object
#if 1
	// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
	if (document.Parse(json).HasParseError())
		return 1;
#else
	// In-situ parsing, decode strings directly in the source string. Source must be string.
	{
		char buffer[sizeof(json)];
		memcpy(buffer, json, sizeof(json));
		if (document.ParseInsitu(buffer).HasParseError())
			return 1;
	}
#endif
	printf("\nParsing succeeded.\n");
	////////////////////////////////////////////////////////////////////////////

	assert(document.IsObject()); // testing dict here
	assert(document.HasMember("dict"));
	assert(document["dict"].IsObject());
	assert(document["dict"].HasMember("s"));
	assert(document["dict"]["s"].IsString());
	assert(document["dict"].HasMember("arr"));
	assert(document["dict"]["arr"].IsArray());
	Value& arr = document["dict"]["arr"];
	printf("dict.arr size: %d\n", arr.Size());
	for (SizeType i = 0; i < arr.Size(); i++) {
		if (arr[i].IsInt()) {
			printf("a[%d] = %d\n", i, arr[i].GetInt());
		} else {
			printf("a[%d] = \"%s\"\n", i, arr[i].GetString());
		}
	}
	puts("Putting new dict in array.\n");
	Document::AllocatorType& allocator = document.GetAllocator();
	Value dict;
	Value insertstr;
	insertstr.SetString("testvalue", allocator);
	dict.SetObject();
	dict.AddMember("testkey", insertstr, allocator);
	arr.PushBack(dict, allocator);
	
	Value newint;
	newint.SetInt(12345);
	document["i"] = newint;
	puts("Done.\n");
	// ->Assertion failed in []:
	//   Value& nonexisting = document["dict"]["NONEXISTING"];
	
#if 0
	// 2. Access values in document.
	printf("\nAccess values in document:\n");
	assert(document.IsObject()); // Document is a JSON value represents the root of DOM. Root can be either an object or array.
	assert(document.HasMember("hello"));
	assert(document["hello"].IsString());
	printf("hello = %s\n", document["hello"].GetString());
	// Since version 0.2, you can use single lookup to check the existing of member and its value:
	Value::MemberIterator hello = document.FindMember("hello");
	assert(hello != document.MemberEnd());
	assert(hello->value.IsString());
	assert(strcmp("world", hello->value.GetString()) == 0);
	(void)hello;
	assert(document["t"].IsBool()); // JSON true/false are bool. Can also uses more specific function IsTrue().
	printf("t = %s\n", document["t"].GetBool() ? "true" : "false");
	assert(document["f"].IsBool());
	printf("f = %s\n", document["f"].GetBool() ? "true" : "false");
	printf("n = %s\n", document["n"].IsNull() ? "null" : "?");
	assert(document["i"].IsNumber()); // Number is a JSON type, but C++ needs more specific type.
	assert(document["i"].IsInt()); // In this case, IsUint()/IsInt64()/IsUInt64() also return true.
	printf("i = %d\n", document["i"].GetInt()); // Alternative (int)document["i"]
	assert(document["pi"].IsNumber());
	assert(document["pi"].IsDouble());
	printf("pi = %g\n", document["pi"].GetDouble());
	{
		const Value& a = document["a"]; // Using a reference for consecutive access is handy and faster.
		assert(a.IsArray());
		for (SizeType i = 0; i < a.Size(); i++) // rapidjson uses SizeType instead of size_t.
			printf("a[%d] = %d\n", i, a[i].GetInt());
		int y = a[0].GetInt();
		(void)y;
		// Iterating array with iterators
		printf("a = ");
		for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
			printf("%d ", itr->GetInt());
		printf("\n");
	}
	// Iterating object members
	static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
		printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
	////////////////////////////////////////////////////////////////////////////
	// 3. Modify values in document.
	// Change i to a bigger number
	{
		uint64_t f20 = 1; // compute factorial of 20
		for (uint64_t j = 1; j <= 20; j++)
			f20 *= j;
		document["i"] = f20; // Alternate form: document["i"].SetUint64(f20)
		assert(!document["i"].IsInt()); // No longer can be cast as int or uint.
	}
	// Adding values to array.
	{
		Value& a = document["a"]; // This time we uses non-const reference.
		Document::AllocatorType& allocator = document.GetAllocator();
		for (int i = 5; i <= 10; i++)
			a.PushBack(i, allocator); // May look a bit strange, allocator is needed for potentially realloc. We normally uses the document's.
		// Fluent API
		a.PushBack("Lua", allocator).PushBack("Mio", allocator);
	}
	// Making string values.
	// This version of SetString() just store the pointer to the string.
	// So it is for literal and string that exists within value's life-cycle.
	{
		document["hello"] = "rapidjson"; // This will invoke strlen()
		// Faster version:
		// document["hello"].SetString("rapidjson", 9);
	}
	// This version of SetString() needs an allocator, which means it will allocate a new buffer and copy the the string into the buffer.
	Value author;
	{
		char buffer[10];
		int len = sprintf(buffer, "%s %s", "Milo", "Yip"); // synthetic example of dynamically created string.
		author.SetString(buffer, static_cast<size_t>(len), document.GetAllocator());
		// Shorter but slower version:
		// document["hello"].SetString(buffer, document.GetAllocator());
		// Constructor version:
		// Value author(buffer, len, document.GetAllocator());
		// Value author(buffer, document.GetAllocator());
		memset(buffer, 0, sizeof(buffer)); // For demonstration purpose.
	}
	// Variable 'buffer' is unusable now but 'author' has already made a copy.
	document.AddMember("author", author, document.GetAllocator());
	assert(author.IsNull()); // Move semantic for assignment. After this variable is assigned as a member, the variable becomes null.
	////////////////////////////////////////////////////////////////////////////
#endif
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	document.Accept(writer);
	std::string docstring = buffer.GetString();
	std::cout << "FIRST DUMP:  " << docstring << std::endl;

	//// 4. Stringify JSON
	//printf("\nModified JSON with reformatting:\n");
	//FileStream f(stdout);
	//PrettyWriter<FileStream> writer(f);
	//document.Accept(writer); // Accept() traverses the DOM and generates Handler events.
	return 0;
}
