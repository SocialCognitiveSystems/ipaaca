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

#ifndef __ipaaca_payload_h_INCLUDED__
#define __ipaaca_payload_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif

// casting operators from Value&
IPAACA_HEADER_EXPORT template<typename T> T json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<typename T> T json_value_cast(const rapidjson::Value* value) { if (!value) return T(); return json_value_cast<T>(*value); }
IPAACA_HEADER_EXPORT template<> long json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> double json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> bool json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> std::string json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> std::vector<std::string> json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> std::list<std::string> json_value_cast(const rapidjson::Value&);
IPAACA_HEADER_EXPORT template<> std::map<std::string, std::string> json_value_cast(const rapidjson::Value&);

// helpers to set Value& from various standard types
IPAACA_HEADER_EXPORT template<typename T> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, T t);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, long);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, double);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, bool);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const std::string&);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const char*);

IPAACA_HEADER_EXPORT template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::vector<T>& ts)
{
	valueobject.SetArray();
	for (auto& val: ts) {
		rapidjson::Value newv;
		pack_into_json_value<T>(newv, allocator, val);
		valueobject.PushBack(newv, allocator);
	}
}
IPAACA_HEADER_EXPORT template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::list<T>& ts)
{
	valueobject.SetArray();
	for (auto& val: ts) {
		rapidjson::Value newv;
		pack_into_json_value<T>(newv, allocator, val);
		valueobject.PushBack(newv, allocator);
	}
}
IPAACA_HEADER_EXPORT template<typename T> void pack_into_json_value(rapidjson::Value& valueobject, rapidjson::Document::AllocatorType& allocator, const std::map<std::string, T>& ts)
{
	valueobject.SetObject();
	for (auto& val: ts) {
		rapidjson::Value key;
		key.SetString(val.first, allocator);
		rapidjson::Value newv;
		pack_into_json_value<T>(newv, allocator, val.second);
		valueobject.AddMember(key, newv, allocator);
	}
}
/*IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const std::vector<std::string>&);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const std::list<std::string>&);
IPAACA_HEADER_EXPORT template<> void pack_into_json_value(rapidjson::Value&, rapidjson::Document::AllocatorType&, const std::map<std::string, std::string>&);
*/

// FIXME TODO locking / invalidating proxy on first write of a payload entry
IPAACA_HEADER_EXPORT class PayloadDocumentEntry//{{{
{
	public:
		IPAACA_MEMBER_VAR_EXPORT ipaaca::Lock lock;
		IPAACA_MEMBER_VAR_EXPORT bool modified;
		IPAACA_MEMBER_VAR_EXPORT std::string json_source;
		IPAACA_MEMBER_VAR_EXPORT rapidjson::Document document;
		IPAACA_HEADER_EXPORT inline PayloadDocumentEntry(): modified(false) { IPAACA_INFO("") }
		IPAACA_HEADER_EXPORT inline ~PayloadDocumentEntry() { IPAACA_INFO("") }
		//IPAACA_HEADER_EXPORT PayloadDocumentEntry(const std::string& source): modified(false), json_source(source), {};
		IPAACA_HEADER_EXPORT std::string to_json_string_representation();
		IPAACA_HEADER_EXPORT static std::shared_ptr<PayloadDocumentEntry> from_json_string_representation(const std::string& input);
		IPAACA_HEADER_EXPORT static std::shared_ptr<PayloadDocumentEntry> create_null();
		IPAACA_HEADER_EXPORT std::shared_ptr<PayloadDocumentEntry> clone();
		IPAACA_HEADER_EXPORT rapidjson::Value& get_or_create_nested_value_from_proxy_path(PayloadEntryProxy* pep);
	typedef std::shared_ptr<PayloadDocumentEntry> ptr;
};
//}}}

/*
IPAACA_HEADER_EXPORT class LegacyStringPayloadEntryProxy//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Payload* _payload;
		IPAACA_MEMBER_VAR_EXPORT std::string _key;
	public:
		IPAACA_HEADER_EXPORT PayloadEntryProxy(Payload* payload, const std::string& key);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const std::string& value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const char* value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(double value);
		IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(bool value);
		IPAACA_HEADER_EXPORT operator std::string();
		IPAACA_HEADER_EXPORT operator long();
		IPAACA_HEADER_EXPORT operator double();
		IPAACA_HEADER_EXPORT operator bool();
		IPAACA_HEADER_EXPORT std::string to_str();
		//long to_int() { return operator long(); ;
		IPAACA_HEADER_EXPORT long to_long();
		IPAACA_HEADER_EXPORT double to_float();
		IPAACA_HEADER_EXPORT bool to_bool();
		// getters
		IPAACA_HEADER_EXPORT template<typename T> T get(); // specializations below
		// setters
};
// Available interpretations of payload entries (or children thereof) below.
//  Usage of standard complex data structures (vector etc.) currently entails
//  casting all entries to a uniform type (a-priori choice: std::string).
IPAACA_HEADER_EXPORT template<> long PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> double PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> bool PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::string PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::vector<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::list<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::map<std::string, std::string> PayloadEntryProxy::get();

//}}}
*/

typedef std::map<std::string, PayloadDocumentEntry::ptr> PayloadDocumentStore;

/*
IPAACA_HEADER_EXPORT class PayloadDocumentStore//{{{
: public std::map<std::string, PayloadDocumentEntry::ptr>
{
	public:
	typedef std::shared_ptr<PayloadDocumentStore> ptr;
};
//}}}
*/
IPAACA_HEADER_EXPORT class Payload//{{{
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
	friend class FakeIU;
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		//IPAACA_MEMBER_VAR_EXPORT rapidjson::Document _json_document;
		//IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rapidjson::Document> _json_store;
		IPAACA_MEMBER_VAR_EXPORT PayloadDocumentStore _document_store;
		IPAACA_MEMBER_VAR_EXPORT boost::weak_ptr<IUInterface> _iu;
	protected:
		IPAACA_HEADER_EXPORT void initialize(boost::shared_ptr<IUInterface> iu);
		IPAACA_HEADER_EXPORT inline void _set_owner_name(const std::string& name) { _owner_name = name; }
		IPAACA_HEADER_EXPORT void _remotely_enforced_wipe();
		IPAACA_HEADER_EXPORT void _remotely_enforced_delitem(const std::string& k);
		IPAACA_HEADER_EXPORT void _remotely_enforced_setitem(const std::string& k, PayloadDocumentEntry::ptr entry);
		//IPAACA_HEADER_EXPORT void _internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_replace_all(const std::map<std::string, PayloadDocumentEntry::ptr>& new_contents, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_merge(const std::map<std::string, PayloadDocumentEntry::ptr>& contents_to_merge, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_set(const std::string& k, PayloadDocumentEntry::ptr v, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_remove(const std::string& k, const std::string& writer_name="");
	public:
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() { return _owner_name; }
		// access
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key);
		IPAACA_HEADER_EXPORT operator std::map<std::string, std::string>();
		IPAACA_HEADER_EXPORT inline void set(const std::string& k, PayloadDocumentEntry::ptr entry) { _internal_set(k, entry); }
		IPAACA_HEADER_EXPORT inline void remove(const std::string& k) { _internal_remove(k); }
		// FIXME: json: these two must support a bunch of standard types, not [only] json (users touch them)
		//  to be more precise: types of map<string, T> with T several interesting things (string, list<string>, etc.)
		//IPAACA_HEADER_EXPORT inline void set(const std::map<std::string, const rapidjson::Document&>& all_elems) { _internal_replace_all(all_elems); }
		//IPAACA_HEADER_EXPORT inline void merge(const std::map<std::string, const rapidjson::Document&>& elems_to_merge) { _internal_merge(elems_to_merge); }
	protected:
		IPAACA_HEADER_EXPORT PayloadDocumentEntry::ptr get_entry(const std::string& k); // json, changed str to proxy here, too
	public:
		IPAACA_HEADER_EXPORT std::string get(const std::string& k); // DEPRECATED
	typedef boost::shared_ptr<Payload> ptr;
};//}}}

IPAACA_HEADER_EXPORT class PayloadEntryProxy//{{{
{
	protected:
	public:
		//IPAACA_MEMBER_VAR_EXPORT rapidjson::Document* _json_parent_node;
		//IPAACA_MEMBER_VAR_EXPORT rapidjson::Document* _json_node;
		IPAACA_MEMBER_VAR_EXPORT Payload* _payload;
		IPAACA_MEMBER_VAR_EXPORT std::string _key;
		//
		// new json stuff / hierarchical navigation
		//
		IPAACA_MEMBER_VAR_EXPORT PayloadEntryProxy* parent; // parent (up to document root -> then null)
		IPAACA_MEMBER_VAR_EXPORT PayloadDocumentEntry::ptr document_entry; // contains lock and json Doc
		IPAACA_MEMBER_VAR_EXPORT bool existent; // whether Value exists already (or blindly navigated)
		IPAACA_MEMBER_VAR_EXPORT bool addressed_as_array; // whether long or string navigation used
		IPAACA_MEMBER_VAR_EXPORT long addressed_index;
		IPAACA_MEMBER_VAR_EXPORT std::string addressed_key;
		/// currently navigated value in json tree (or a new Null value)
		IPAACA_MEMBER_VAR_EXPORT rapidjson::Value* json_value;
/*	protected:
		IPAACA_HEADER_EXPORT void connect_to_existing_parents();
*/
	public:
		// constructor to create a new top-most parent proxy (from a payload key)
		IPAACA_HEADER_EXPORT PayloadEntryProxy(Payload* payload, const std::string& key);
		// constructors for navigation through objects
		IPAACA_HEADER_EXPORT PayloadEntryProxy(PayloadEntryProxy* parent, const std::string& addressed_key);
		IPAACA_HEADER_EXPORT PayloadEntryProxy(PayloadEntryProxy* parent, size_t addressed_index);
	public:
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](size_t index); // array-style navigation
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key);
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const char* key);
		//                   
		IPAACA_HEADER_EXPORT template<typename T> PayloadEntryProxy& operator=(T t)
		{
			PayloadDocumentEntry::ptr new_entry = document_entry->clone(); // copy-on-write, no lock required
			rapidjson::Value& newval = new_entry->get_or_create_nested_value_from_proxy_path(this);
			pack_into_json_value(newval, new_entry->document.GetAllocator(), t);
			_payload->set(_key, new_entry);
			return *this;
		}
		
		//IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const std::string& value);
		//IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(const char* value);
		//IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(double value);
		//IPAACA_HEADER_EXPORT PayloadEntryProxy& operator=(bool value);
		
		IPAACA_HEADER_EXPORT operator std::string();
		IPAACA_HEADER_EXPORT operator long();
		IPAACA_HEADER_EXPORT operator double();
		IPAACA_HEADER_EXPORT operator bool();
		IPAACA_HEADER_EXPORT template<typename Inner> operator std::vector<Inner>() {
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			std::vector<Inner> result;
			for (auto it = json_value->Begin(); it != json_value->End(); ++it) {
				result.push_back( json_value_cast<Inner>(*it) );
			}
			return result;
		}
		IPAACA_HEADER_EXPORT template<typename Inner> operator std::list<Inner>() {
			if ((!json_value) || (!json_value->IsArray())) throw PayloadAddressingError();
			std::list<Inner> result;
			for (auto it = json_value->Begin(); it != json_value->End(); ++it) {
				result.push_back( json_value_cast<Inner>(*it) );
			}
			return result;
		}
		IPAACA_HEADER_EXPORT template<typename Inner> operator std::map<std::string, Inner>() {
			if ((!json_value) || (!json_value->IsObject())) throw PayloadAddressingError();
			std::map<std::string, Inner> result;
			for (auto it = json_value->MemberBegin(); it != json_value->MemberEnd(); ++it) {
				result[std::string(it->name.GetString())] = json_value_cast<Inner>(it->value);
			}
			return result;
		}
		// FIXME why are these needed again?
		IPAACA_HEADER_EXPORT std::string to_str();
		//long to_int() { return operator long(); ;
		IPAACA_HEADER_EXPORT long to_long();
		IPAACA_HEADER_EXPORT double to_float();
		IPAACA_HEADER_EXPORT bool to_bool();
		// getters
		IPAACA_HEADER_EXPORT template<typename T> T get() { return json_value_cast<T>(json_value); } // specializations below
		// setters
		IPAACA_HEADER_EXPORT template<typename T> PayloadEntryProxy& set(T t);
		/*{
			pack_into_json_value<T>(t);
			connect_to_existing_parents();
			_payload->set(key, document_entry->document);
		}*/
};
// Available interpretations of payload entries (or children thereof) below.
//  Usage of standard complex data structures (vector etc.) currently entails
//  casting all entries to a uniform type (a-priori choice: std::string).
/*
IPAACA_HEADER_EXPORT template<> long PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> double PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> bool PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::string PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::vector<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::list<std::string> PayloadEntryProxy::get();
IPAACA_HEADER_EXPORT template<> std::map<std::string, std::string> PayloadEntryProxy::get();
*/

//}}}

#endif
