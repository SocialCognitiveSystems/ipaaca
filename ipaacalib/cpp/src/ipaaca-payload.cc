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
		os << "\"" << kv.first << "\":" << kv.second->to_json_string_representation() << "";
	}
	os << "}";
	return os;
}
//}}}

double strict_numerical_interpretation(const std::string& str)
{
	char* endptr;
	auto s = str_trim(str);
	const char* startptr = s.c_str();
	double d = strtod(startptr, &endptr);
	if ((*endptr)=='\0') {
		// everything could be parsed
		return d;
	} else {
		throw PayloadTypeConversionError();
	}
}

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
	if (v.IsString()) return (long) strict_numerical_interpretation(v.GetString());
	if (v.IsInt()) return v.GetInt();
	if (v.IsUint()) return v.GetUint();
	if (v.IsInt64()) return v.GetInt64();
	if (v.IsUint64()) return v.GetUint64();
	if (v.IsDouble()) return (long) v.GetDouble();
	if (v.IsBool()) return v.GetBool() ? 1l : 0l;
	if (v.IsNull()) return 0l;
	// default: return parse of string version (should always be 0 though?)
	throw PayloadTypeConversionError();
}
IPAACA_EXPORT template<> int json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return (int) strict_numerical_interpretation(v.GetString());
	if (v.IsInt()) return v.GetInt();
	if (v.IsUint()) return v.GetUint();
	if (v.IsInt64()) return v.GetInt64();
	if (v.IsUint64()) return v.GetUint64();
	if (v.IsDouble()) return (long) v.GetDouble();
	if (v.IsBool()) return v.GetBool() ? 1l : 0l;
	if (v.IsNull()) return 0l;
	throw PayloadTypeConversionError();
}
IPAACA_EXPORT template<> double json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) return strict_numerical_interpretation(v.GetString());
	if (v.IsDouble()) return v.GetDouble();
	if (v.IsInt()) return (double) v.GetInt();
	if (v.IsUint()) return (double) v.GetUint();
	if (v.IsInt64()) return (double) v.GetInt64();
	if (v.IsUint64()) return (double) v.GetUint64();
	if (v.IsBool()) return v.GetBool() ? 1.0 : 0.0;
	if (v.IsNull()) return 0.0;
	throw PayloadTypeConversionError();
}
IPAACA_EXPORT template<> bool json_value_cast(const rapidjson::Value& v)
{
	if (v.IsString()) {
		std::string s = v.GetString();
		return !((s=="")||(s=="false")||(s=="False")||(s=="0"));
	}
	if (v.IsBool()) return v.GetBool();
	if (v.IsNull()) return false;
	if (v.IsInt()) return v.GetInt() != 0;
	if (v.IsUint()) return v.GetUint() != 0;
	if (v.IsInt64()) return v.GetInt64() != 0;
	if (v.IsUint64()) return v.GetUint64() != 0;
	if (v.IsDouble()) return v.GetDouble() != 0.0;
	// default: assume "pointer-like" semantics (i.e. objects are TRUE)
	return true;
}
//}}}

IPAACA_EXPORT void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, int newvalue)
{
	valueobject.SetInt(newvalue);
}
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
	return entry;
}
IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::from_unquoted_string_value(const std::string& str)
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	entry->document.SetString(str.c_str(), entry->document.GetAllocator());
	return entry;
}

IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::create_null()
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	return entry;
}
IPAACA_EXPORT PayloadDocumentEntry::ptr PayloadDocumentEntry::clone()
{
	auto entry = PayloadDocumentEntry::create_null();
	entry->document.CopyFrom(this->document, entry->document.GetAllocator());
	IPAACA_DEBUG("PayloadDocumentEntry cloned for copy-on-write, contents: " << entry)
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
			IPAACA_INFO("parent value is not of type Array")
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
	} else {
		IPAACA_DEBUG("Addressed as dict with key " << pep->addressed_key)
		if (! parent_value.IsObject()) {
			IPAACA_INFO("parent value is not of type Object")
			throw PayloadAddressingError();
		} else {
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
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
		IPAACA_INFO("Invalid json_value")
		throw PayloadAddressingError();
	}
	if (! json_value->IsObject()) {
		IPAACA_INFO("Expected Object for operator[](string)")
		throw PayloadAddressingError();
	}
	return PayloadEntryProxy(this, addr_key_);
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](size_t addr_idx_)
{
	if (!json_value) {
		IPAACA_INFO("Invalid json_value")
		throw PayloadAddressingError();
	}
	if (! json_value->IsArray()) {
		IPAACA_INFO("Expected Array for operator[](size_t)")
		throw PayloadAddressingError();
	}
	long s = json_value->Size();
	if (addr_idx_>=s) {
		IPAACA_INFO("Array out of bounds")
		throw PayloadAddressingError();
	}
	return PayloadEntryProxy(this, addr_idx_);
}
IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxy::operator[](int addr_idx_)
{
	if (addr_idx_ < 0) {
		IPAACA_INFO("Negative array index")
		throw PayloadAddressingError();
	}
	return operator[]((size_t) addr_idx_);
}

IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(const PayloadEntryProxy& otherproxy)
{
	PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
	rapidjson::Value& newval = new_entry->get_or_create_nested_value_from_proxy_path(this);
	auto valueptr = otherproxy.json_value;
	if (valueptr) { // only set if value is valid, keep default null value otherwise
		newval.CopyFrom(*valueptr, new_entry->document.GetAllocator());
	}
	_payload->set(_key, new_entry);
	return *this;
}

IPAACA_EXPORT PayloadEntryProxy::operator std::string()
{
	return json_value_cast<std::string>(json_value);
}
IPAACA_EXPORT PayloadEntryProxy::operator long()
{
	return json_value_cast<long>(json_value);
}
IPAACA_EXPORT PayloadEntryProxy::operator double()
{
	return json_value_cast<double>(json_value);
}
IPAACA_EXPORT PayloadEntryProxy::operator bool()
{
	return json_value_cast<bool>(json_value);
}
IPAACA_EXPORT std::string PayloadEntryProxy::to_str()
{
	return json_value_cast<std::string>(json_value);
}
IPAACA_EXPORT long PayloadEntryProxy::to_long()
{
	return json_value_cast<long>(json_value);
}
IPAACA_EXPORT double PayloadEntryProxy::to_float()
{
	return json_value_cast<double>(json_value);
}
IPAACA_EXPORT bool PayloadEntryProxy::to_bool()
{
	return json_value_cast<bool>(json_value);
}

IPAACA_EXPORT PayloadEntryProxyMapDecorator PayloadEntryProxy::as_map()
{
	if (json_value && json_value->IsObject()) return PayloadEntryProxyMapDecorator(this);
	throw PayloadTypeConversionError();
}

IPAACA_EXPORT PayloadEntryProxyListDecorator PayloadEntryProxy::as_list()
{
	if (json_value && json_value->IsArray()) return PayloadEntryProxyListDecorator(this);
	throw PayloadTypeConversionError();
}

IPAACA_EXPORT size_t PayloadEntryProxy::size()
{
	if (!json_value) return 0;
	if (json_value->IsArray()) return json_value->Size();
	if (json_value->IsObject()) return json_value->MemberCount();
	return 0;
}
IPAACA_EXPORT bool PayloadEntryProxy::is_null()
{
	return (!json_value) || json_value->IsNull();
}
IPAACA_EXPORT bool PayloadEntryProxy::is_string()
{
	return json_value && json_value->IsString();
}

/// is_number => whether it is *interpretable* as a numerical value (i.e. including conversions)
IPAACA_EXPORT bool PayloadEntryProxy::is_number()
{
	if (!json_value) return false;
	try {
		double dummy = json_value_cast<double>(*json_value);
		return true;
	} catch (PayloadTypeConversionError& ex) {
		return false;
	}
}
IPAACA_EXPORT bool PayloadEntryProxy::is_list()
{
	return json_value && json_value->IsArray();
}
IPAACA_EXPORT bool PayloadEntryProxy::is_map()
{
	return json_value && json_value->IsObject();
}

//}}}

// Payload//{{{

IPAACA_EXPORT void Payload::on_lock()
{
	Locker locker(_payload_operation_mode_lock);
	IPAACA_DEBUG("Starting payload batch update mode ...")
	_update_on_every_change = false;
}
IPAACA_EXPORT void Payload::on_unlock()
{
	Locker locker(_payload_operation_mode_lock);
	IPAACA_DEBUG("... applying payload batch update with " << _collected_modifications.size() << " modifications and " << _collected_removals.size() << " removals ...")
	_internal_merge_and_remove(_collected_modifications, _collected_removals, _batch_update_writer_name);
	_update_on_every_change = true;
	_batch_update_writer_name = "";
	_collected_modifications.clear();
	_collected_removals.clear();
	IPAACA_DEBUG("... exiting payload batch update mode.")
}

IPAACA_EXPORT void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = boost::weak_ptr<IUInterface>(iu);
}

IPAACA_EXPORT PayloadEntryProxy Payload::operator[](const std::string& key)
{
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
	Locker locker(_payload_operation_mode_lock);
	if (_update_on_every_change) {
		std::map<std::string, PayloadDocumentEntry::ptr> _new;
		std::vector<std::string> _remove;
		_new[k] = v;
		_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
		IPAACA_DEBUG(" Setting local payload item \"" << k << "\" to " << v)
		_document_store[k] = v;
		mark_revision_change();
	} else {
		IPAACA_DEBUG("queueing a payload set operation")
		_batch_update_writer_name = writer_name;
		_collected_modifications[k] = v;
		// revoke deletions of this updated key
		std::vector<std::string> new_removals;
		for (auto& rk: _collected_removals) {
			if (rk!=k) new_removals.push_back(rk);
		}
		_collected_removals = new_removals;
	}
}
IPAACA_EXPORT void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	Locker locker(_payload_operation_mode_lock);
	if (_update_on_every_change) {
		std::map<std::string, PayloadDocumentEntry::ptr> _new;
		std::vector<std::string> _remove;
		_remove.push_back(k);
		_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
		_document_store.erase(k);
		mark_revision_change();
	} else {
		IPAACA_DEBUG("queueing a payload remove operation")
		_batch_update_writer_name = writer_name;
		_collected_removals.push_back(k);
		// revoke updates of this deleted key
		_collected_modifications.erase(k);
	}
}
IPAACA_EXPORT void Payload::_internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name)
{
	Locker locker(_payload_operation_mode_lock);
	if (_update_on_every_change) {
		std::vector<std::string> _remove;
		_iu.lock()->_modify_payload(false, new_contents, _remove, writer_name );
		_document_store = new_contents;
		mark_revision_change();
	} else {
		IPAACA_DEBUG("queueing a payload replace_all operation")
		_batch_update_writer_name = writer_name;
		_collected_modifications.clear();
		for (auto& kv: new_contents) {
			_collected_modifications[kv.first] = kv.second;
		}
		// take all existing keys and flag to remove them, unless overridden in current update
		for (auto& kv: _document_store) {
			if (! new_contents.count(kv.first)) {
				_collected_removals.push_back(kv.first);
				_collected_modifications.erase(kv.first);
			}
		}
	}
}
IPAACA_EXPORT void Payload::_internal_merge(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::string& writer_name)
{
	Locker locker(_payload_operation_mode_lock);
	if (_update_on_every_change) {
		std::vector<std::string> _remove;
		_iu.lock()->_modify_payload(true, contents_to_merge, _remove, writer_name );
		for (auto& kv: contents_to_merge) {
			_document_store[kv.first] = kv.second;
		}
		mark_revision_change();
	} else {
		IPAACA_DEBUG("queueing a payload merge operation")
		std::set<std::string> updated_keys;
		_batch_update_writer_name = writer_name;
		for (auto& kv: contents_to_merge) {
			_collected_modifications[kv.first] = kv.second;
			updated_keys.insert(kv.first);
		}
		// revoke deletions of updated keys
		std::vector<std::string> new_removals;
		for (auto& rk: _collected_removals) {
			if (! updated_keys.count(rk)) new_removals.push_back(rk);
		}
		_collected_removals = new_removals;
	}
}
IPAACA_EXPORT void Payload::_internal_merge_and_remove(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	// this function is called by exiting the batch update mode only, so no extra locking here
	_iu.lock()->_modify_payload(true, contents_to_merge, keys_to_remove, writer_name );
	for (auto& k: keys_to_remove) {
		_document_store.erase(k);
	}
	for (auto& kv: contents_to_merge) {
		_document_store[kv.first] = kv.second;
	}
	mark_revision_change();
}
IPAACA_EXPORT PayloadDocumentEntry::ptr Payload::get_entry(const std::string& k) {
	if (_document_store.count(k)>0) return _document_store[k];
	else return PayloadDocumentEntry::create_null();  // contains Document with 'null' value
}
IPAACA_EXPORT std::string Payload::get(const std::string& k) { // DEPRECATED
	if (_document_store.count(k)>0) return _document_store[k]->document.GetString();
	return "";
}

IPAACA_EXPORT void Payload::set(const std::map<std::string, std::string>& all_elems)
{
	std::map<std::string, PayloadDocumentEntry::ptr> newmap;
	for (auto& kv: all_elems) {
		newmap[kv.first] = PayloadDocumentEntry::from_unquoted_string_value(kv.second);
	}
	_internal_replace_all(newmap);
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
IPAACA_EXPORT std::shared_ptr<std::pair<std::string, PayloadEntryProxy> > PayloadIterator::operator->()
{
	if (_payload->revision_changed(reference_payload_revision)) throw PayloadIteratorInvalidError();
	if (raw_iterator == _payload->_document_store.end()) throw PayloadIteratorInvalidError();
	return std::make_shared<std::pair<std::string, PayloadEntryProxy> >(raw_iterator->first, PayloadEntryProxy(_payload, raw_iterator->first));
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

// PayloadEntryProxyMapIterator//{{{
IPAACA_EXPORT PayloadEntryProxyMapIterator::PayloadEntryProxyMapIterator(PayloadEntryProxy* proxy_, RawIterator&& raw_iter)
: proxy(proxy_), raw_iterator(std::move(raw_iter))
{
}

IPAACA_EXPORT PayloadEntryProxyMapIterator& PayloadEntryProxyMapIterator::operator++()
{
	raw_iterator++;
	return *this;
}

IPAACA_EXPORT std::pair<std::string, PayloadEntryProxy> PayloadEntryProxyMapIterator::operator*()
{
	std::string key = raw_iterator->name.GetString();
	return std::pair<std::string, PayloadEntryProxy>(key, (*proxy)[key] ); // generates child Proxy
}

IPAACA_EXPORT std::shared_ptr<std::pair<std::string, PayloadEntryProxy> > PayloadEntryProxyMapIterator::operator->()
{
	std::string key = raw_iterator->name.GetString();
	return std::make_shared<std::pair<std::string, PayloadEntryProxy> >(key, (*proxy)[key] ); // generates child Proxy
}
IPAACA_EXPORT bool PayloadEntryProxyMapIterator::operator==(const PayloadEntryProxyMapIterator& other_iter)
{
	return raw_iterator==other_iter.raw_iterator;
}
IPAACA_EXPORT bool PayloadEntryProxyMapIterator::operator!=(const PayloadEntryProxyMapIterator& other_iter)
{
	return raw_iterator!=other_iter.raw_iterator;
}
//}}}
// PayloadEntryProxyMapDecorator//{{{
PayloadEntryProxyMapIterator PayloadEntryProxyMapDecorator::begin()
{
	return PayloadEntryProxyMapIterator(proxy, proxy->json_value->MemberBegin());
}
PayloadEntryProxyMapIterator PayloadEntryProxyMapDecorator::end()
{
	return PayloadEntryProxyMapIterator(proxy, proxy->json_value->MemberEnd());
}
//}}}

// PayloadEntryProxyListIterator//{{{
IPAACA_EXPORT PayloadEntryProxyListIterator::PayloadEntryProxyListIterator(PayloadEntryProxy* proxy_, size_t idx, size_t size_)
: proxy(proxy_), current_idx(idx), size(size_)
{
}

IPAACA_EXPORT PayloadEntryProxyListIterator& PayloadEntryProxyListIterator::operator++()
{
	if (current_idx!=size) current_idx++;
	return *this;
}

IPAACA_EXPORT PayloadEntryProxy PayloadEntryProxyListIterator::operator*()
{
	return (*proxy)[current_idx];
}

IPAACA_EXPORT std::shared_ptr<PayloadEntryProxy> PayloadEntryProxyListIterator::operator->()
{
	return std::make_shared<PayloadEntryProxy>((*proxy)[current_idx]);
}
IPAACA_EXPORT bool PayloadEntryProxyListIterator::operator==(const PayloadEntryProxyListIterator& other_iter)
{
	return (proxy==other_iter.proxy) && (current_idx==other_iter.current_idx);
}
IPAACA_EXPORT bool PayloadEntryProxyListIterator::operator!=(const PayloadEntryProxyListIterator& other_iter)
{
	return (current_idx!=other_iter.current_idx) || (proxy!=other_iter.proxy);
}
//}}}
// PayloadEntryProxyListDecorator//{{{
PayloadEntryProxyListIterator PayloadEntryProxyListDecorator::begin()
{
	return PayloadEntryProxyListIterator(proxy, 0, proxy->json_value->Size());
}
PayloadEntryProxyListIterator PayloadEntryProxyListDecorator::end()
{
	size_t size = proxy->json_value->Size();
	return PayloadEntryProxyListIterator(proxy, size, size);
}
//}}}

} // of namespace ipaaca
