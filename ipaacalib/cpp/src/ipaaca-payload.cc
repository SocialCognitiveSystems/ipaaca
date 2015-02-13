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

namespace ipaaca {

using namespace rapidjson;

using namespace rsb;
using namespace rsb::filter;
using namespace rsb::converter;
using namespace rsb::patterns;

IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const Payload& obj)//{{{
{
	os << "{";
	bool first = true;
	for (auto& kv: obj._document_store) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << kv.first << "':'" << kv.second->json_source << "'";
	}
	os << "}";
	return os;
}
//}}}

// json_value_cast//{{{
IPAACA_HEADER_EXPORT template<> std::string json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return v.GetString();
	if (v.IsNull()) return "";
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	v.Accept(writer);
	return buffer.GetString();
}
IPAACA_HEADER_EXPORT template<> long json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return atol(std::string(v.GetString()).c_str());
	if (v.IsInt()) return v.GetInt();
	if (v.IsUint()) return v.GetUint();
	if (v.IsInt64()) return v.GetInt64();
	if (v.IsUint64()) return v.GetUint64();
	if (v.IsDouble()) return (long) v.GetDouble();
	if (v.IsBool()) return v.GetBool() ? 1l : 0l;
	if (v.IsNull()) return 0l;
	// default: return parse of string version (should always be 0 though?)
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	v.Accept(writer);
	return atol(std::string(buffer.GetString()).c_str());
}
IPAACA_HEADER_EXPORT template<> double json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return atof(std::string(v.GetString()).c_str());
	if (v.IsDouble()) return v.GetDouble();
	if (v.IsInt()) return (double) v.GetInt();
	if (v.IsUint()) return (double) v.GetUint();
	if (v.IsInt64()) return (double) v.GetInt64();
	if (v.IsUint64()) return (double) v.GetUint64();
	if (v.IsBool()) return v.GetBool() ? 1.0 : 0.0;
	if (v.IsNull()) return 0.0;
	// default: return parse of string version (should always be 0.0 though?)
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	v.Accept(writer);
	return atof(std::string(buffer.GetString()).c_str());
}
IPAACA_HEADER_EXPORT template<> bool json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) {
		std::string s = v.GetString();
		return !((s=="")||(s=="false")||(s=="False")||(s=="0"));
		//return ((s=="1")||(s=="true")||(s=="True"));
	}
	if (v.IsBool()) return v.GetBool();
	if (v.IsNull()) return false;
	if (v.IsInt()) return v.GetInt() != 0;
	if (v.IsUint()) return v.GetUint() != 0;
	if (v.IsInt64()) return v.GetInt64() != 0;
	if (v.IsUint64()) return v.GetUint64() != 0;
	if (v.IsDouble()) return v.GetDouble() != 0.0;
	// default: return parse of string version (should always be 0.0 though?)
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	v.Accept(writer);
	std::string s = buffer.GetString();
	return !((s=="")||(s=="false")||(s=="False")||(s=="0"));
}
/*
 * std::map<std::string, std::string> result;
	std::for_each(_document_store.begin(), _document_store.end(), [&result](std::pair<std::string, PayloadDocumentEntry::ptr> pair) {
			result[pair.first] =  pair.second->document.GetString();
			});
			*/
//}}}

// PayloadDocumentEntry//{{{
IPAACA_HEADER_EXPORT inline std::string PayloadDocumentEntry::to_json_string_representation()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);
	return buffer.GetString();
}
IPAACA_HEADER_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::from_json_string_representation(const std::string& json_str)
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	if (entry->document.Parse(json_str.c_str()).HasParseError()) {
		throw JsonParsingError();
	}
	entry->json_source = json_str;
	return entry;
}
IPAACA_HEADER_EXPORT inline PayloadDocumentEntry::ptr PayloadDocumentEntry::create_null()
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	entry->json_source = "null";
	return entry;
}
//}}}

// PayloadEntryProxy//{{{

 // only if not top-level
IPAACA_EXPORT void PayloadEntryProxy::connect_to_existing_parents()
{
	rapidjson::Document::AllocatorType& allocator = document_entry->document.GetAllocator();
	PayloadEntryProxy* pep = this;
	while (!(pep->existent) && pep->parent) { // only if not top-level
		if (pep->addressed_as_array) {
			rapidjson::Value& parent_value = *(pep->parent->json_value);
			if (! parent_value.IsArray()) {
				throw PayloadAddressingError();
			} else {
				long idx = pep->addressed_index;
				long s = parent_value.Size();
				if (idx<s) {
					parent_value[idx] = *json_value;
				} else {
					throw PayloadAddressingError();
				}
			}
			/*if (parent_value.IsNull()) {
				wasnull = true;
				parent_value.SetArray();
			}
			if (wasnull || parent_value.IsArray()) {
				long idx = pep->addressed_index;
				long s = parent_value.Size();
				if (idx<s) {
					// existing element modified
					parent_value[idx] = *json_value;
				} else {
					// implicitly initialize missing elements to null values
					if (idx>s) {
						long missing_elements = pep->addressed_index - p;
						for (int i=0; i<missing_elements; ++i) {
							parent_value.PushBack(, allocator)
						}
					}
				}
				if (s == 
			} else {
				throw PayloadAddressingError();
			}*/
		} else {
			// addressed as object (dict)
			rapidjson::Value& parent_value = *(pep->parent->json_value);
			if (! parent_value.IsObject()) {
				throw PayloadAddressingError();
			} else {
				Value key;
				key.SetString(pep->addressed_key, allocator);
				parent_value.AddMember(key, *json_value, allocator);
			}
		}
		// repeat for next parent in the tree
		pep = pep->parent;
	}
}



IPAACA_EXPORT PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key)
: _payload(payload), _key(key), parent(nullptr)
{
	document_entry = _payload->get_entry(key);
	json_value = &(document_entry->document);
}
IPAACA_EXPORT PayloadEntryProxy::PayloadEntryProxy(PayloadEntryProxy* parent_, const std::string& addr_key_)
: parent(parent_), addressed_key(addr_key_), addressed_as_array(false)
{
	_payload = parent->_payload;
	_key = parent->_key;
	document_entry = parent->document_entry;
	auto it = parent->json_value->FindMember(addr_key_.c_str());
	if (it != json_value->MemberEnd()) {
		json_value = &(parent->json_value->operator[](addr_key_.c_str()));
		existent = true;
	} else {
		json_value = nullptr; // avoid heap construction here
		existent = false;
	}
}
IPAACA_EXPORT PayloadEntryProxy::PayloadEntryProxy(PayloadEntryProxy* parent_, size_t addr_idx_)
: parent(parent_), addressed_index(addr_idx_), addressed_as_array(true)
{
	_payload = parent->_payload;
	_key = parent->_key;
	document_entry = parent->document_entry;
	json_value = &(parent->json_value->operator[](addr_idx_));
	existent = true;
}

IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](const std::string& addr_key_)
{
	if (!json_value) throw PayloadAddressingError();
	if (! json_value->IsObject()) throw PayloadAddressingError();
	return PayloadEntryProxy(this, addr_key_);
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](size_t addr_idx_)
{
	if (!json_value) throw PayloadAddressingError();
	if (! json_value->IsArray()) throw PayloadAddressingError();
	long s = json_value->Size();
	if (addr_idx_>=s) throw PayloadAddressingError();
	return PayloadEntryProxy(this, addr_idx_);
}

IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(const std::string& value)
{
	//std::cout << "operator=(string)" << std::endl;
	IPAACA_IMPLEMENT_ME
	//_payload->set(_key, value);
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(const char* value)
{
	//std::cout << "operator=(const char*)" << std::endl;
	IPAACA_IMPLEMENT_ME
	//_payload->set(_key, value);
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(double value)
{
	//std::cout << "operator=(double)" << std::endl;
	IPAACA_IMPLEMENT_ME
	//_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(bool value)
{
	//std::cout << "operator=(bool)" << std::endl;
	IPAACA_IMPLEMENT_ME
	//_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}

IPAACA_EXPORT PayloadEntryProxy::operator std::string()
{
	return PayloadEntryProxy::get<std::string>();
}
IPAACA_EXPORT PayloadEntryProxy::operator long()
{
	return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT PayloadEntryProxy::operator double()
{
	return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT PayloadEntryProxy::operator bool()
{
	return PayloadEntryProxy::get<bool>();
}
IPAACA_EXPORT std::string PayloadEntryProxy::to_str()
{
	return PayloadEntryProxy::get<std::string>(); 
}
IPAACA_EXPORT long PayloadEntryProxy::to_long()
{
	return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT double PayloadEntryProxy::to_float()
{
	return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT bool PayloadEntryProxy::to_bool()
{
	return PayloadEntryProxy::get<bool>();
}


std::string value_diagnosis(rapidjson::Value* val)
{
	if (!val) return "nullptr";
	if (val->IsNull()) return "null";
	if (val->IsString()) return "string";
	if (val->IsNumber()) return "number";
	if (val->IsBool()) return "bool";
	if (val->IsArray()) return "array";
	if (val->IsObject()) return "object";
	return "other";

}

//
// new stuff for protocol v2
//

/*
IPAACA_HEADER_EXPORT template<> std::string PayloadEntryProxy::get<std::string>()
{
	if (!json_value) return "";
	//IPAACA_INFO( value_diagnosis(json_value) )
	if (json_value->IsString()) return json_value->GetString();
	if (json_value->IsNull()) return "";
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	json_value->Accept(writer);
	return buffer.GetString();
	
	//return _payload->get(_key);
}
IPAACA_HEADER_EXPORT template<> long PayloadEntryProxy::get<long>()
{
	return atof(operator std::string().c_str());
}
IPAACA_HEADER_EXPORT template<> double PayloadEntryProxy::get<double>()
{
	return atol(operator std::string().c_str());
}
IPAACA_HEADER_EXPORT template<> bool PayloadEntryProxy::get<bool>()
{
	std::string s = operator std::string();
	return ((s=="1")||(s=="true")||(s=="True"));
}
// complex types
IPAACA_HEADER_EXPORT template<> std::list<std::string> PayloadEntryProxy::get<std::list<std::string> >()
{
	std::list<std::string> l;
	l.push_back(PayloadEntryProxy::get<std::string>());
	return l;
}
IPAACA_HEADER_EXPORT template<> std::vector<std::string> PayloadEntryProxy::get<std::vector<std::string> >()
{
	std::vector<std::string> v;
	v.push_back(PayloadEntryProxy::get<std::string>());
	return v;
}
IPAACA_HEADER_EXPORT template<> std::map<std::string, std::string> PayloadEntryProxy::get<std::map<std::string, std::string> >()
{
	std::map<std::string, std::string> m;
	m["__automatic__"] = PayloadEntryProxy::get<std::string>();
	return m;
}
*/

//}}}

// Payload//{{{

IPAACA_EXPORT void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = boost::weak_ptr<IUInterface>(iu);
}

IPAACA_EXPORT PayloadEntryProxy Payload::operator[](const std::string& key)
{
	// TODO atomicize
	//boost::shared_ptr<PayloadEntryProxy> p(new PayloadEntryProxy(this, key));
	return PayloadEntryProxy(this, key);
}
IPAACA_EXPORT Payload::operator std::map<std::string, std::string>()
{
	std::map<std::string, std::string> result;
	std::for_each(_document_store.begin(), _document_store.end(), [&result](std::pair<std::string, PayloadDocumentEntry::ptr> pair) {
			result[pair.first] =  pair.second->document.GetString();
			});
	return result;
}

IPAACA_EXPORT void Payload::_internal_set(const std::string& k, PayloadDocumentEntry::ptr v, const std::string& writer_name) {
	std::map<std::string, PayloadDocumentEntry::ptr> _new;
	std::vector<std::string> _remove;
	_new[k] = v;
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_document_store[k] = v;
}
IPAACA_EXPORT void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	std::map<std::string, PayloadDocumentEntry::ptr> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_document_store.erase(k);
}
IPAACA_EXPORT void Payload::_internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(false, new_contents, _remove, writer_name );
	_document_store = new_contents;
}
IPAACA_EXPORT void Payload::_internal_merge(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(true, contents_to_merge, _remove, writer_name );
	_document_store.insert(contents_to_merge.begin(), contents_to_merge.end());
	//for (std::map<std::string, std::string>::iterator it = contents_to_merge.begin(); it!=contents_to_merge.end(); i++) {
	//	_store[it->first] = it->second;
	//}
}
IPAACA_EXPORT inline PayloadDocumentEntry::ptr Payload::get_entry(const std::string& k) {
	if (_document_store.count(k)>0) return _document_store[k];
	else return PayloadDocumentEntry::create_null();  // contains Document with 'null' value
}
IPAACA_EXPORT inline std::string Payload::get(const std::string& k) { // DEPRECATED
	if (_document_store.count(k)>0) return _document_store[k]->document.GetString();
	return "";
}
IPAACA_EXPORT void Payload::_remotely_enforced_wipe()
{
	_document_store.clear();
}
IPAACA_EXPORT void Payload::_remotely_enforced_delitem(const std::string& k)
{
	_document_store.erase(k);
}
IPAACA_EXPORT void Payload::_remotely_enforced_setitem(const std::string& k, PayloadDocumentEntry::ptr entry)
{
	_document_store[k] = entry;
}

//}}}

} // of namespace ipaaca
