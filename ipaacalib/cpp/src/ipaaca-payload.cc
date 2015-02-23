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

// temporary helper to show rapidjson internal type
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


IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const rapidjson::Value& val)//{{{
{
	os << json_value_cast<std::string>(val);
	return os;
}
//}}}
IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const PayloadEntryProxy& proxy)//{{{
{
	if (proxy.json_value) os << *(proxy.json_value);
	else os << "null";
	return os;
}
//}}}
IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, PayloadDocumentEntry::ptr entry)//{{{
{
	os << json_value_cast<std::string>(entry->document);
	return os;
}
//}}}

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
IPAACA_EXPORT template<> std::string json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return v.GetString();
	if (v.IsNull()) return "";
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	v.Accept(writer);
	return buffer.GetString();
}
IPAACA_EXPORT template<> long json_value_cast(const rapidjson::Value& v)
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
IPAACA_EXPORT template<> double json_value_cast(const rapidjson::Value& v)
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
IPAACA_EXPORT template<> bool json_value_cast(const rapidjson::Value& v)
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

IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, long newvalue)
{
	valueobject.SetInt(newvalue);
}
IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, double newvalue)
{
	valueobject.SetDouble(newvalue);
}
IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, bool newvalue)
{
	valueobject.SetBool(newvalue);
}
IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::string& newvalue)
{
	valueobject.SetString(newvalue.c_str(), allocator);
}
IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const char* newvalue)
{
	valueobject.SetString(newvalue, allocator);
}
/*
IPAACA_EXPORT template<> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::vector<std::string>& newvalue)
{
	valueobject.SetArray();
	for (auto& str: newvalue) {
		rapidjson::Value sv;
		sv.SetString(str, allocator);
		valueobject.PushBack(sv, allocator);
	}
}
IPAACA_EXPORT template<> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::list<std::string>& newvalue)
{
	IPAACA_IMPLEMENT_ME
}
IPAACA_EXPORT template<> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::map<std::string, std::string>& newvalue)
{
	IPAACA_IMPLEMENT_ME
}
*/

// PayloadDocumentEntry//{{{
IPAACA_EXPORT std::string PayloadDocumentEntry::to_json_string_representation()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);
	return buffer.GetString();
}
IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::from_json_string_representation(const std::string& json_str)
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	if (entry->document.Parse(json_str.c_str()).HasParseError()) {
		throw JsonParsingError();
	}
	entry->json_source = json_str;
	return entry;
}

/// update json_source after a write operation (on newly cloned entries)
IPAACA_EXPORT void PayloadDocumentEntry::update_json_source()
{
	json_source = to_json_string_representation();
}


IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::create_null()
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	entry->json_source = "null"; // rapidjson::Document value is also null implicitly
	return entry;
}
IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::clone()
{
	auto entry = PayloadDocumentEntry::from_json_string_representation(this->json_source);
	IPAACA_DEBUG("Cloned for copy-on-write, contents: " << entry)
	return entry;
}
IPAACA_EXPORT rapidjson::Value& PayloadDocumentEntry::get_or_create_nested_value_from_proxy_path(PayloadEntryProxy* pep)
{
	if (!(pep->parent)) {
		return document;
	}
	rapidjson::Value& parent_value = get_or_create_nested_value_from_proxy_path(pep->parent);
	if (pep->addressed_as_array) {
		IPAACA_DEBUG("Addressed as array with index " << pep->addressed_index)
		if (! parent_value.IsArray()) {
			throw PayloadAddressingError();
		} else {
			long idx = pep->addressed_index;
			long s = parent_value.Size();
			if (idx<s) {
				return parent_value[idx];
			} else {
				throw PayloadAddressingError();
			}
		}
		// for append / push_back? :
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
		IPAACA_DEBUG("Addressed as dict with key " << pep->addressed_key)
		// addressed as object (dict)
		//rapidjson::Value& parent_value = *(pep->parent->json_value);
		if (! parent_value.IsObject()) {
			IPAACA_DEBUG("parent is not of type Object")
			throw PayloadAddressingError();
		} else {
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
			//Value key;
			//key.SetString(pep->addressed_key, allocator);
			//parent_value.AddMember(key, *json_value, allocator);
			rapidjson::Value key;
			key.SetString(pep->addressed_key, allocator);
			auto it = parent_value.FindMember(key);
			if (it != parent_value.MemberEnd()) {
				return parent_value[key];
			} else {
				rapidjson::Value val;
				parent_value.AddMember(key, val, allocator);
				rapidjson::Value rkey;
				rkey.SetString(pep->addressed_key, allocator);
				return parent_value[rkey];
			}
		}
	}
}

//}}}

// PayloadEntryProxy//{{{

 // only if not top-level
#if 0
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
#endif


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
	if (it != parent->json_value->MemberEnd()) {
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

IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](const char* addr_key_)
{
	return operator[](std::string(addr_key_));
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](const std::string& addr_key_)
{
	if (!json_value) {
		IPAACA_DEBUG("Invalid json_value!")
		throw PayloadAddressingError();
	}
	if (! json_value->IsObject()) {
		IPAACA_DEBUG("Expected Object for operator[](string)!")
		throw PayloadAddressingError();
	}
	return PayloadEntryProxy(this, addr_key_);
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](size_t addr_idx_)
{
	if (!json_value) {
		IPAACA_DEBUG("Invalid json_value!")
		throw PayloadAddressingError();
	}
	if (! json_value->IsArray()) {
		IPAACA_DEBUG("Expected Array for operator[](size_t)!")
		throw PayloadAddressingError();
	}
	long s = json_value->Size();
	if (addr_idx_>=s) {
		IPAACA_DEBUG("Array out of bounds!")
		throw PayloadAddressingError();
	}
	return PayloadEntryProxy(this, addr_idx_);
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](int addr_idx_)
{
	if (addr_idx_ < 0) {
		IPAACA_DEBUG("Negative index!")
		throw PayloadAddressingError();
	}
	return operator[]((size_t) addr_idx_);
}

/*
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
*/

IPAACA_EXPORT PayloadEntryProxy::operator std::string()
{
	return json_value_cast<std::string>(json_value);
	//PayloadEntryProxy::get<std::string>();
}
IPAACA_EXPORT PayloadEntryProxy::operator long()
{
	return json_value_cast<long>(json_value);
	//return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT PayloadEntryProxy::operator double()
{
	return json_value_cast<double>(json_value);
	//return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT PayloadEntryProxy::operator bool()
{
	return json_value_cast<bool>(json_value);
	//return PayloadEntryProxy::get<bool>();
}
IPAACA_EXPORT std::string PayloadEntryProxy::to_str()
{
	return json_value_cast<std::string>(json_value);
	//return PayloadEntryProxy::get<std::string>(); 
}
IPAACA_EXPORT long PayloadEntryProxy::to_long()
{
	return json_value_cast<long>(json_value);
	//return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT double PayloadEntryProxy::to_float()
{
	return json_value_cast<double>(json_value);
	//return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT bool PayloadEntryProxy::to_bool()
{
	return json_value_cast<bool>(json_value);
	//return PayloadEntryProxy::get<bool>();
}


//
// new stuff for protocol v2
//

/*
IPAACA_EXPORT template<> std::string PayloadEntryProxy::get<std::string>()
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
IPAACA_EXPORT template<> long PayloadEntryProxy::get<long>()
{
	return atof(operator std::string().c_str());
}
IPAACA_EXPORT template<> double PayloadEntryProxy::get<double>()
{
	return atol(operator std::string().c_str());
}
IPAACA_EXPORT template<> bool PayloadEntryProxy::get<bool>()
{
	std::string s = operator std::string();
	return ((s=="1")||(s=="true")||(s=="True"));
}
// complex types
IPAACA_EXPORT template<> std::list<std::string> PayloadEntryProxy::get<std::list<std::string> >()
{
	std::list<std::string> l;
	l.push_back(PayloadEntryProxy::get<std::string>());
	return l;
}
IPAACA_EXPORT template<> std::vector<std::string> PayloadEntryProxy::get<std::vector<std::string> >()
{
	std::vector<std::string> v;
	v.push_back(PayloadEntryProxy::get<std::string>());
	return v;
}
IPAACA_EXPORT template<> std::map<std::string, std::string> PayloadEntryProxy::get<std::map<std::string, std::string> >()
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
			result[pair.first] =  json_value_cast<std::string>(pair.second->document);
			});
	return result;
}

IPAACA_EXPORT void Payload::_internal_set(const std::string& k, PayloadDocumentEntry::ptr v, const std::string& writer_name) {
	std::map<std::string, PayloadDocumentEntry::ptr> _new;
	std::vector<std::string> _remove;
	_new[k] = v;
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	IPAACA_DEBUG(" Setting local payload item \"" << k << "\" to " << v)
	_document_store[k] = v;
	mark_revision_change();
}
IPAACA_EXPORT void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	std::map<std::string, PayloadDocumentEntry::ptr> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_document_store.erase(k);
	mark_revision_change();
}
IPAACA_EXPORT void Payload::_internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(false, new_contents, _remove, writer_name );
	_document_store = new_contents;
	mark_revision_change();
}
IPAACA_EXPORT void Payload::_internal_merge(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(true, contents_to_merge, _remove, writer_name );
	for (auto& kv: contents_to_merge) {
		_document_store[kv.first] = kv.second;
	}
	mark_revision_change();
	//_document_store.insert(contents_to_merge.begin(), contents_to_merge.end());
	//for (std::map<std::string, std::string>::iterator it = contents_to_merge.begin(); it!=contents_to_merge.end(); i++) {
	//	_store[it->first] = it->second;
	//}
}
IPAACA_EXPORT PayloadDocumentEntry::ptr Payload::get_entry(const std::string& k) {
	if (_document_store.count(k)>0) return _document_store[k];
	else return PayloadDocumentEntry::create_null();  // contains Document with 'null' value
}
IPAACA_EXPORT std::string Payload::get(const std::string& k) { // DEPRECATED
	if (_document_store.count(k)>0) return _document_store[k]->document.GetString();
	return "";
}
IPAACA_EXPORT void Payload::_remotely_enforced_wipe()
{
	_document_store.clear();
	mark_revision_change();
}
IPAACA_EXPORT void Payload::_remotely_enforced_delitem(const std::string& k)
{
	_document_store.erase(k);
	mark_revision_change();
}
IPAACA_EXPORT void Payload::_remotely_enforced_setitem(const std::string& k, PayloadDocumentEntry::ptr entry)
{
	_document_store[k] = entry;
	mark_revision_change();
}
IPAACA_EXPORT PayloadIterator Payload::begin()
{
	return PayloadIterator(this, _document_store.begin());
}
IPAACA_EXPORT PayloadIterator Payload::end()
{
	return PayloadIterator(this, _document_store.end());
}

//}}}

// PayloadIterator//{{{
IPAACA_EXPORT PayloadIterator::PayloadIterator(Payload* payload, PayloadDocumentStore::iterator&& ref_it)
: _payload(payload), reference_payload_revision(payload->internal_revision), raw_iterator(std::move(ref_it))
{
}
IPAACA_EXPORT PayloadIterator::PayloadIterator(const PayloadIterator& iter)
: _payload(iter._payload), reference_payload_revision(iter.reference_payload_revision), raw_iterator(iter.raw_iterator)
{
}

IPAACA_EXPORT PayloadIterator& PayloadIterator::operator++()
{
	if (_payload->revision_changed(reference_payload_revision)) throw PayloadIteratorInvalidError();
	++raw_iterator;
	return *this;
}

IPAACA_EXPORT std::pair<std::string, PayloadEntryProxy> PayloadIterator::operator*()
{
	if (_payload->revision_changed(reference_payload_revision)) throw PayloadIteratorInvalidError();
	if (raw_iterator == _payload->_document_store.end()) throw PayloadIteratorInvalidError();
	return std::pair<std::string, PayloadEntryProxy>(raw_iterator->first, PayloadEntryProxy(_payload, raw_iterator->first));
}

IPAACA_EXPORT bool PayloadIterator::operator==(const PayloadIterator& ref)
{
	if (_payload->revision_changed(reference_payload_revision)) throw PayloadIteratorInvalidError();
	return (raw_iterator==ref.raw_iterator);
}
IPAACA_EXPORT bool PayloadIterator::operator!=(const PayloadIterator& ref)
{
	if (_payload->revision_changed(reference_payload_revision)) throw PayloadIteratorInvalidError();
	return (raw_iterator!=ref.raw_iterator);
}
//}}}

} // of namespace ipaaca
