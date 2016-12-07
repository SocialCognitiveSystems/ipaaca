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

/**
 * \file   ipaaca-payload.h
 *
 * \brief Header file for IU payload functionality.
 *
 * Users should not include this file directly, but use ipaaca.h
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#ifndef __ipaaca_payload_h_INCLUDED__
#define __ipaaca_payload_h_INCLUDED__

#include <typeinfo>

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif

// casting operators from Value&
/// 'Smart' type conversions, allowing for some leeway type-wise (e.g. string "1.3" can be successfully cast to double or long). Used by PayloadEntryProxy.
template<typename T> T json_value_cast(const rapidjson::Value&);
template<typename T> T json_value_cast(const rapidjson::Value* value) { if (!value) return T(); return json_value_cast<T>(*value); }
template<> IPAACA_HEADER_EXPORT long json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT int json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT double json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT bool json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT std::string json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT std::vector<std::string> json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT std::list<std::string> json_value_cast(const rapidjson::Value&);
template<> IPAACA_HEADER_EXPORT std::map<std::string, std::string> json_value_cast(const rapidjson::Value&);

// helpers to set Value& from various standard types
//IPAACA_HEADER_EXPORT template<typename T> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, T t);
/// Setter to store int into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, int);
/// Setter to store int into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, long);
/// Setter to store long into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, double);
/// Setter to store double into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, bool);
/// Setter to store bool into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const std::string&);
/// Setter to store std::string into json value, used by PayloadEntryProxy.
IPAACA_HEADER_EXPORT void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const char*);
// helpers to set Value& from several standard containers containing the above standard types
/// Setter to store a vector of supported basic types into json value, used by PayloadEntryProxy.
template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::vector<T>& ts)
{
	valueobject.SetArray();
	for (auto& val: ts) {
		rapidjson::Value newv;
		pack_into_json_value(newv, allocator, val);
		valueobject.PushBack(newv, allocator);
	}
}
/// Setter to store a list of supported basic types into json value, used by PayloadEntryProxy.
template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::list<T>& ts)
{
	valueobject.SetArray();
	for (auto& val: ts) {
		rapidjson::Value newv;
		pack_into_json_value(newv, allocator, val);
		valueobject.PushBack(newv, allocator);
	}
}
/// Setter to store a map of string -> supported basic types into json value, used by PayloadEntryProxy.
template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::map<std::string, T>& ts)
{
	valueobject.SetObject();
	for (auto& val: ts) {
		rapidjson::Value key;
		key.SetString(val.first, allocator);
		rapidjson::Value newv;
		pack_into_json_value(newv, allocator, val.second);
		valueobject.AddMember(key, newv, allocator);
	}
}

/// Single payload entry wrapping a rapidjson::Document with some conversion glue. Also handles copy-on-write Document cloning. <b>Internal type</b> - users generally do not see this.
class PayloadDocumentEntry//{{{
{
	friend std::ostream& operator<<(std::ostream& os, std::shared_ptr<PayloadDocumentEntry> entry);
	public:
		IPAACA_MEMBER_VAR_EXPORT ipaaca::Lock lock;
		IPAACA_MEMBER_VAR_EXPORT bool modified;
		IPAACA_MEMBER_VAR_EXPORT rapidjson::Document document;
		IPAACA_HEADER_EXPORT inline PayloadDocumentEntry(): modified(false) { }
		IPAACA_HEADER_EXPORT inline ~PayloadDocumentEntry() { }
		IPAACA_HEADER_EXPORT std::string to_json_string_representation();
		IPAACA_HEADER_EXPORT static std::shared_ptr<PayloadDocumentEntry> from_json_string_representation(const std::string& input);
		IPAACA_HEADER_EXPORT static std::shared_ptr<PayloadDocumentEntry> from_unquoted_string_value(const std::string& input);
		IPAACA_HEADER_EXPORT static std::shared_ptr<PayloadDocumentEntry> create_null();
		IPAACA_HEADER_EXPORT std::shared_ptr<PayloadDocumentEntry> clone();
		IPAACA_HEADER_EXPORT rapidjson::Value& get_or_create_nested_value_from_proxy_path(PayloadEntryProxy* pep);
	typedef std::shared_ptr<PayloadDocumentEntry> ptr;
};
//}}}

typedef std::map<std::string, PayloadDocumentEntry::ptr> PayloadDocumentStore;


/** \brief Central class containing the user-set payload of any IUInterface class (IU, Message, RemotePushIU or RemoteMessage)
 *
 * Obtained by calling payload() on any IUInterface derived object. Created during IU creation.
 */
class Payload: public Lock //{{{
{
	friend std::ostream& operator<<(std::ostream& os, const Payload& obj);
	friend class IUInterface;
	friend class IU;
	friend class Message;
	friend class RemotePushIU;
	friend class RemoteMessage;
	friend class IUConverter;
	friend class MessageConverter;
	friend class CallbackIUPayloadUpdate;
	friend class PayloadEntryProxy;
	friend class PayloadIterator;
	friend class FakeIU;
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		IPAACA_MEMBER_VAR_EXPORT PayloadDocumentStore _document_store;
		IPAACA_MEMBER_VAR_EXPORT boost::weak_ptr<IUInterface> _iu;
		IPAACA_MEMBER_VAR_EXPORT Lock _payload_operation_mode_lock; //< enforcing atomicity wrt the bool flag below
		IPAACA_MEMBER_VAR_EXPORT bool _update_on_every_change; //< true: batch update not active; false: collecting updates (payload locked)
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, PayloadDocumentEntry::ptr> _collected_modifications;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> _collected_removals; // TODO use set later? cf. IU, too
		IPAACA_MEMBER_VAR_EXPORT std::string _batch_update_writer_name;
	protected:
		/// inherited from ipaaca::Lock, starting batch update collection mode
		IPAACA_HEADER_EXPORT void on_lock() override;
		/// inherited from ipaaca::Lock, finishing batch update collection mode
		IPAACA_HEADER_EXPORT void on_unlock() override;
		/// thread ID for current write access (to let that thread read from cache and others from old payload)
		IPAACA_MEMBER_VAR_EXPORT std::string _writing_thread_id;
	protected:
		IPAACA_HEADER_EXPORT void initialize(boost::shared_ptr<IUInterface> iu);
		IPAACA_HEADER_EXPORT inline void _set_owner_name(const std::string& name) { _owner_name = name; }
		IPAACA_HEADER_EXPORT void _remotely_enforced_wipe();
		IPAACA_HEADER_EXPORT void _remotely_enforced_delitem(const std::string& k);
		IPAACA_HEADER_EXPORT void _remotely_enforced_setitem(const std::string& k, PayloadDocumentEntry::ptr entry);
		IPAACA_HEADER_EXPORT void _internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_merge(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_set(const std::string& k, PayloadDocumentEntry::ptr v, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_remove(const std::string& k, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_merge_and_remove(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="");
	public:
		IPAACA_HEADER_EXPORT inline Payload(): _batch_update_writer_name(""), _update_on_every_change(true) { }
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() { return _owner_name; }
		// access
		/// Obtain a payload item by name as a PayloadEntryProxy (returning null-type proxy if undefined)
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key);
		/// Legacy / convenience function: interpret the payload map as a map string->string (casting all entries to string)
		IPAACA_HEADER_EXPORT operator std::map<std::string, std::string>();
		/// remove a single payload entry
		IPAACA_HEADER_EXPORT inline void remove(const std::string& k) { _internal_remove(k); }
		/// Legacy / convenience function: set the whole payload map from a map string->string (all JSON types are also set as string, no interpretation)
		IPAACA_HEADER_EXPORT void set(const std::map<std::string, std::string>& all_elems);
	protected:
		/// set or overwrite a single payload entry with a PayloadDocumentEntry object (used by PayloadEntryProxy::operator=()).
		IPAACA_HEADER_EXPORT inline void set(const std::string& k, PayloadDocumentEntry::ptr entry) { _internal_set(k, entry); }
		IPAACA_HEADER_EXPORT PayloadDocumentEntry::ptr get_entry(const std::string& k); // json, changed str to proxy here, too
	public:
		[[deprecated("Use operator[] and operator std::string() instead")]]
		/// Read a single entry as string [DEPRECATED] (use string conversion in PayloadEntryProxy instead)
		IPAACA_HEADER_EXPORT std::string get(const std::string& k);
	protected:
		IPAACA_MEMBER_VAR_EXPORT unsigned long internal_revision;
		IPAACA_MEMBER_VAR_EXPORT inline void mark_revision_change() { internal_revision++; }
		IPAACA_HEADER_EXPORT inline bool revision_changed(unsigned long reference_revision) { return internal_revision != reference_revision; }
	public:
		/// obtain a standard iterator marking the first entry in the payload
		IPAACA_HEADER_EXPORT PayloadIterator begin();
		/// obtain a standard iterator past the last entry in the payload
		IPAACA_HEADER_EXPORT PayloadIterator end();
	typedef boost::shared_ptr<Payload> ptr;
};//}}}

/** \brief Standard iterator for Payload (example below)
 *
 * \b Examples:
 * <pre>
 * // Print all key-value pairs from a payload (C++11)
 * for (auto kv_pair: myiu->payload()) {
 *     std::cout << kv_pair.first << " -> " << (std::string) kv_pair.second << std::endl;
 * }
 * </pre>
 *
 */
class PayloadIterator//{{{
{
	friend class Payload;
	friend std::ostream& operator<<(std::ostream& os, const PayloadIterator& iter);
	protected:
		IPAACA_MEMBER_VAR_EXPORT Payload* _payload;
		IPAACA_MEMBER_VAR_EXPORT unsigned long reference_payload_revision;
		IPAACA_MEMBER_VAR_EXPORT PayloadDocumentStore::iterator raw_iterator;
	protected:
		IPAACA_HEADER_EXPORT PayloadIterator(Payload* payload, PayloadDocumentStore::iterator&& pl_iter ); //, bool is_end);
	public:
		IPAACA_HEADER_EXPORT PayloadIterator(const PayloadIterator& iter);
		IPAACA_HEADER_EXPORT PayloadIterator& operator++();
		IPAACA_HEADER_EXPORT std::pair<std::string, PayloadEntryProxy> operator*();
		IPAACA_HEADER_EXPORT std::shared_ptr<std::pair<std::string, PayloadEntryProxy> > operator->();
		IPAACA_HEADER_EXPORT bool operator==(const PayloadIterator& ref);
		IPAACA_HEADER_EXPORT bool operator!=(const PayloadIterator& ref);
};
//}}}

/// Iterator over a payload entry that is a json map-type object (returned type during dereferencing: pair<string, PayloadEntryProxy>)
class PayloadEntryProxyMapIterator//{{{
{
	public:
		typedef rapidjson::GenericDocument<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>, rapidjson::CrtAllocator>::MemberIterator RawIterator;
	protected:
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* proxy;
		IPAACA_MEMBER_VAR_EXPORT RawIterator raw_iterator;
	public:
		IPAACA_HEADER_EXPORT PayloadEntryProxyMapIterator(PayloadEntryProxy* proxy, RawIterator&& raw_iter);
		IPAACA_HEADER_EXPORT PayloadEntryProxyMapIterator& operator++();
		IPAACA_HEADER_EXPORT std::pair<std::string, PayloadEntryProxy> operator*();
		IPAACA_HEADER_EXPORT std::shared_ptr<std::pair<std::string, PayloadEntryProxy> > operator->();
		IPAACA_HEADER_EXPORT bool operator==(const PayloadEntryProxyMapIterator& other_iter);
		IPAACA_HEADER_EXPORT bool operator!=(const PayloadEntryProxyMapIterator& other_iter);
};
//}}}
/// Iterator over a payload entry that is a json list-type object (returned type during dereferencing: PayloadEntryProxy)
class PayloadEntryProxyListIterator//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* proxy;
		IPAACA_MEMBER_VAR_EXPORT size_t current_idx;
		IPAACA_MEMBER_VAR_EXPORT size_t size;
	public:
		IPAACA_HEADER_EXPORT PayloadEntryProxyListIterator(PayloadEntryProxy* proxy, size_t idx, size_t size);
		IPAACA_HEADER_EXPORT PayloadEntryProxyListIterator& operator++();
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator*();
		IPAACA_HEADER_EXPORT std::shared_ptr<PayloadEntryProxy> operator->();
		IPAACA_HEADER_EXPORT bool operator==(const PayloadEntryProxyListIterator& other_iter);
		IPAACA_HEADER_EXPORT bool operator!=(const PayloadEntryProxyListIterator& other_iter);
};
//}}}
/// Interpretation of a variant json value as a map-type object
class PayloadEntryProxyMapDecorator//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline PayloadEntryProxyMapDecorator(PayloadEntryProxy* proxy_): proxy(proxy_) { }
		IPAACA_HEADER_EXPORT PayloadEntryProxyMapIterator begin();
		IPAACA_HEADER_EXPORT PayloadEntryProxyMapIterator end();
	protected:
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* proxy;
};
//}}}
/// Interpretation of a variant json value as a list-type object
class PayloadEntryProxyListDecorator//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline PayloadEntryProxyListDecorator(PayloadEntryProxy* proxy_): proxy(proxy_) { }
		IPAACA_HEADER_EXPORT PayloadEntryProxyListIterator begin();
		IPAACA_HEADER_EXPORT PayloadEntryProxyListIterator end();
	protected:
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* proxy;
};
//}}}

/** \brief Reference to an existent or nonexistent payload entry (or a value deeper in the json tree)
 *
 * This class is returned by IUInterface::operator[].
 * The proxy handles automatic type conversions, requests remote changes of payloads, and enables navigation into and iteration over structured json objects.
 *
 * \b Examples (reading):
 *
 * <code>std::string received_name = iu->payload()["name"];</code>  // implicit conversion using operator string()
 *
 * <code>std::vector<double> vd = iu->payload()["double_list"];</code>  // some standard container types also supported
 * 
 * <code>auto p = iu->payload()["otherKey"];</code>  // auto type is PayloadEntryProxy (conversion is on-demand)
 *
 * <code>for (auto val: iu->payload()["my_list"].as_list()) { ... }</code>  // as_list is required to select list-type iteration (value type in iteration remains variant)
 *
 * <code>for (auto k_v_pair: iu->payload()["my_map"].as_map()) { ... }</code>  // as_map is required to select map-type iteration (value type in iteration is a pair, second part remains variant)
 *
 * \b Examples (writing):
 * 
 * <code>iu->payload()["my_new_item"] = "new value";</code>  // most basic operation, set string value
 *
 * <code>iu->payload()["double_list"][0] = 100.0;</code>  // accessing and updating an item in a list
 *
 * <code>iu->payload()["name_list"] = std::list<std::string>{"Alpha", "Bravo", "Charlie"};</code>  // set from basic uniform containers
 *
 * <code>iu->payload()["name_list"].push_back("--- adding some numbers below ---");</code>  // append a supported value to an existing list
 *
 * <code>iu->payload()["name_list"].extend(iu->payload()["double_list"]);</code>  // extend list by items; \b Note: all setters also accept proxies as source values, creating copies of values
 */
class PayloadEntryProxy//{{{
{
	friend class Payload;
	friend class PayloadIterator;
	friend class PayloadDocumentEntry;
	friend class PayloadEntryProxyListDecorator;
	friend class PayloadEntryProxyListIterator;
	friend class PayloadEntryProxyMapDecorator;
	friend class PayloadEntryProxyMapIterator;
	friend std::ostream& operator<<(std::ostream& os, const PayloadEntryProxy& proxy);
	public:
		/// Select map-style iteration for this proxy (to select iterator content type). Will throw if not actually map-type. See example in the class description.
		IPAACA_HEADER_EXPORT PayloadEntryProxyMapDecorator as_map();
		/// Select list-style iteration for this proxy (to select iterator content type). Will throw if not actually list-type. See example in the class description.
		IPAACA_HEADER_EXPORT PayloadEntryProxyListDecorator as_list();
	protected:
		IPAACA_MEMBER_VAR_EXPORT Payload* _payload;
		IPAACA_MEMBER_VAR_EXPORT std::string _key;
		// new json stuff / hierarchical navigation
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* parent; ///< Parent proxy (up to document root -> then null)
		IPAACA_MEMBER_VAR_EXPORT PayloadDocumentEntry::ptr document_entry; // contains lock and json Doc
		IPAACA_MEMBER_VAR_EXPORT bool existent; ///< Whether Value exists already (or else 'blindly' navigated)
		IPAACA_MEMBER_VAR_EXPORT bool addressed_as_array; ///< Whether long or string navigation was used
		IPAACA_MEMBER_VAR_EXPORT long addressed_index; ///< Index that was used in list-style access
		IPAACA_MEMBER_VAR_EXPORT std::string addressed_key; ///< Key that was used in map-style access
		/// currently navigated value in json tree (or a new Null value)
		IPAACA_MEMBER_VAR_EXPORT rapidjson::Value* json_value; ///< json value that corresponds to the current navigation (or nullptr)
	protected:
		// constructor to create a new top-most parent proxy (from a payload key)
		IPAACA_HEADER_EXPORT PayloadEntryProxy(Payload* payload, const std::string& key);
		// constructors for navigation through objects
		IPAACA_HEADER_EXPORT PayloadEntryProxy(PayloadEntryProxy* parent, const std::string& addressed_key);
		IPAACA_HEADER_EXPORT PayloadEntryProxy(PayloadEntryProxy* parent, size_t addressed_index);
	public:
		/// Return number of contained items (or 0 for non-container types)
		IPAACA_HEADER_EXPORT size_t size();
		/// Return whether value corresponds to json 'null'; also true if value is nonexistent so far (e.g. navigated to new map entry)
		IPAACA_HEADER_EXPORT bool is_null();
		/// Return whether value is of string type
		IPAACA_HEADER_EXPORT bool is_string();
		/// Return whether value is of a numerical type
		IPAACA_HEADER_EXPORT bool is_number();
		/// Return whether value is of list type
		IPAACA_HEADER_EXPORT bool is_list();
		/// Return whether value is of map type
		IPAACA_HEADER_EXPORT bool is_map();
	public:
		/// Array-style navigation over json value
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](size_t index); // array-style navigation
		/// Array-style navigation over json value (added to catch [0])
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](int index); // int is UNFORTUNATELY required to catch
																	  // [0] (addressing using literal zero)
																	  // because ambiguity with const char*
																	  // arises if only [](size_t) is provided.
																	  // size_t is obviously superior ...
																	  // TODO: remove if better solution known
		/// Dict-style navigation over json value
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key); // dict-style navigation
		/// Dict-style navigation over json value
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const char* key);
		//
		/// Set or overwrite some portion of a payload from the point navigated to
		template<typename T> PayloadEntryProxy& operator=(T t)
		{
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& newval = new_entry->get_or_create_nested_value_from_proxy_path(this);
			pack_into_json_value(newval, new_entry->document.GetAllocator(), t);
			_payload->set(_key, new_entry);
			return *this;
		}
		/// Value comparison with other proxy contents
		IPAACA_HEADER_EXPORT inline bool operator==(const PayloadEntryProxy& otherproxy) { return (json_value && otherproxy.json_value && ((*json_value)==*(otherproxy.json_value))); }
		/// Value comparison with other proxy contents
		IPAACA_HEADER_EXPORT inline bool operator!=(const PayloadEntryProxy& otherproxy) { return !operator==(otherproxy); }
		/// Value comparison with supported basic types
		template<typename T> bool operator==(T othervalue)
		{
			if (!json_value) return false;
			try {
				return json_value_cast<T>(*json_value) == othervalue;
			} catch(PayloadTypeConversionError& ex) {
				// assume conversion error = type mismatch = unequal
				return false;
			}
		}
		/// Value comparison with supported basic types
		template<typename T> bool operator!=(T othervalue) { return !operator==(othervalue); }
		/// Value comparison with char* (required to be explicitly added)
		IPAACA_HEADER_EXPORT inline bool operator==(const char* othervalue)
		{
			if (!json_value) return false;
			return json_value_cast<std::string>(*json_value) == othervalue;
		}
		/// Value comparison with char* (required to be explicitly added)
		IPAACA_HEADER_EXPORT inline bool operator!=(const char* othervalue) { return !operator==(othervalue); }
		
		/// Copy value from below other json node, preserving types
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const PayloadEntryProxy& otherproxy);
		
		/// Conversion to std::string (explicit or implicit)
		IPAACA_HEADER_EXPORT operator std::string();
		/// Conversion to long (explicit or implicit)
		IPAACA_HEADER_EXPORT operator long();
		/// Conversion to double (explicit or implicit)
		IPAACA_HEADER_EXPORT operator double();
		/// Conversion to bool (explicit or implicit)
		IPAACA_HEADER_EXPORT operator bool();
		/// Conversion to uniform std::vector of supported basic type
		template<typename Inner> operator std::vector<Inner>() {
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			std::vector<Inner> result;
			for (auto it = json_value->Begin(); it != json_value->End(); ++it) {
				result.push_back( json_value_cast<Inner>(*it) );
			}
			return result;
		}
		/// Conversion to uniform std::list of supported basic type
		template<typename Inner> operator std::list<Inner>() {
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			std::list<Inner> result;
			for (auto it = json_value->Begin(); it != json_value->End(); ++it) {
				result.push_back( json_value_cast<Inner>(*it) );
			}
			return result;
		}
		/// Conversion to uniform std::map of string -> supported basic type
		template<typename Inner> operator std::map<std::string, Inner>() {
			if ((!json_value) || (!json_value->IsObject())) throw PayloadAddressingError();
			std::map<std::string, Inner> result;
			for (auto it = json_value->MemberBegin(); it != json_value->MemberEnd(); ++it) {
				result[std::string(it->name.GetString())] = json_value_cast<Inner>(it->value);
			}
			return result;
		}
		// TODO maybe remove these deprecated converters later
		/// [DECPRECATED] use normal type conversion syntax instead
		[[deprecated("Use operator std::string() instead (i.e. explicit or implicit cast)")]]
		IPAACA_HEADER_EXPORT std::string to_str();
		//long to_int() { return operator long(); ;
		/// [DECPRECATED] use normal type conversion syntax instead
		[[deprecated("Use operator long() instead (i.e. explicit or implicit cast)")]]
		IPAACA_HEADER_EXPORT long to_long();
		/// [DECPRECATED] use normal type conversion syntax instead
		[[deprecated("Use operator double() instead (i.e. explicit or implicit cast)")]]
		IPAACA_HEADER_EXPORT double to_float();
		/// [DECPRECATED] use normal type conversion syntax instead
		[[deprecated("Use operator bool() instead (i.e. explicit or implicit cast)")]]
		IPAACA_HEADER_EXPORT bool to_bool();
		/// Append a supported type to a list-type payload value
		template<typename T> void push_back(T t)
		{
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& list = new_entry->get_or_create_nested_value_from_proxy_path(this);
			rapidjson::Value newval;
			pack_into_json_value(newval, new_entry->document.GetAllocator(), t);
			list.PushBack(newval, new_entry->document.GetAllocator());
			_payload->set(_key, new_entry);
		}
		/// Append the value of another proxy (or a null value) to a list-type value
		IPAACA_HEADER_EXPORT void push_back(const PayloadEntryProxy& otherproxy)
		{
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& list = new_entry->get_or_create_nested_value_from_proxy_path(this);
			rapidjson::Value newval;
			auto valueptr = otherproxy.json_value;
			if (valueptr) { // only set if value is valid, keep default null value otherwise
				newval.CopyFrom(*valueptr, new_entry->document.GetAllocator());
			}
			list.PushBack(newval, new_entry->document.GetAllocator());
			_payload->set(_key, new_entry);
		}
		/// Extend a list-type payload value with a vector containing items of a supported type
		template<typename T> void extend(const std::vector<T>& ts)
		{
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& list = new_entry->get_or_create_nested_value_from_proxy_path(this);
			for (auto& t: ts) {
				rapidjson::Value newval;
				pack_into_json_value(newval, new_entry->document.GetAllocator(), t);
				list.PushBack(newval, new_entry->document.GetAllocator());
			}
			_payload->set(_key, new_entry);
		}
		/// Extend a list-type payload value with a list containing items of a supported type
		template<typename T> void extend(const std::list<T>& ts)
		{
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& list = new_entry->get_or_create_nested_value_from_proxy_path(this);
			for (auto& t: ts) {
				rapidjson::Value newval;
				pack_into_json_value(newval, new_entry->document.GetAllocator(), t);
				list.PushBack(newval, new_entry->document.GetAllocator());
			}
			_payload->set(_key, new_entry);
		}
		/// Extend a list-type payload value with items (copies) from another list-type value
		IPAACA_HEADER_EXPORT void extend(const PayloadEntryProxy& otherproxy)
		{
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			if ((!otherproxy.json_value) || (!(otherproxy.json_value->IsArray()))) throw PayloadAddressingError();
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& list = new_entry->get_or_create_nested_value_from_proxy_path(this);
			for (size_t i=0; i<otherproxy.json_value->Size(); ++i) {
				rapidjson::Value newval;
				rapidjson::Value& value = (*(otherproxy.json_value))[i];
				newval.CopyFrom(value, new_entry->document.GetAllocator());
				list.PushBack(newval, new_entry->document.GetAllocator());
			}
			_payload->set(_key, new_entry);
		}
};

//}}}

#endif
