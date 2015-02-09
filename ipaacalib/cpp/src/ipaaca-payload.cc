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

using namespace rsb;
using namespace rsb::filter;
using namespace rsb::converter;
using namespace rsb::patterns;

IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const Payload& obj)//{{{
{
	os << "{";
	bool first = true;
	for (std::map<std::string, std::string>::const_iterator it=obj._document_store.begin(); it!=obj._document_store.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "':'" << it->second << "'";
	}
	os << "}";
	return os;
}
//}}}

// PayloadDocumentEntry//{{{
IPAACA_HEADER_EXPORT inline std::string PayloadDocumentEntry::to_json_string_representation()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);
	return buffer.GetString();
}
IPAACA_HEADER_EXPORT inline PayloadDocumentEntry::ptr PayloadDocumentEntry::from_json_string_representation(const std::string& json)
{
	PayloadDocumentEntry::ptr entry = std::make_shared<ipaaca::PayloadDocumentEntry>();
	if (entry->document.Parse(json).HasParseError()) {
		throw JsonParsingError();
	}
	return entry;
}
//}}}

// PayloadEntryProxy//{{{

 // only if not top-level
IPAACA_EXPORT void PayloadEntryProxy::connect_to_existing_parents()
{
	rapidjson::Document::AllocatorType& allocator = document_entry->document.GetAllocator();
	PayloadEntryProxy* pep = this;
	while (!(pep->existant) && pep->parent) { // only if not top-level
		if (pep->addressed_as_array) {
			Value& parent_value = pep->parent->json_value;
			if (! parent_value.IsArray()) {
				throw PayloadAddressingError();
			} else {
				long idx = pep->addressed_index;
				long s = parent_value.Size();
				if (idx<s) {
					parent_value[idx] = json_value;
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
					parent_value[idx] = json_value;
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
			Value& parent_value = pep->parent->json_value;
			if (! parent_value.IsObject()) {
				throw PayloadAddressingError();
			} else {
				parent_value.AddMember(pep->addressed_key, json_value, allocator);
			}
		}
		// repeat for next parent in the tree
		pep = pep->parent;
	}
}

IPAACA_EXPORT PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key, PayloadDocumentEntry::ptr entry)
: _payload(payload), _key(key), parent(nullptr), document_entry(entry)
{
	PayloadEntryProxy* parent; // parent (up to document root -> then null)
	PayloadDocumentEntry::ptr document_entry; // contains lock and json Doc
	bool existant; // whether Value exist already (or blindly navigated)
	bool addressed_as_array; // whether long or string navigation used
	long addressed_index;
	std::string addressed_key;
	/// currently navigated value in json tree (or a new Null value)
	rapidjson::Value& json_value;
	
}

IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(const std::string& value)
{
	//std::cout << "operator=(string)" << std::endl;
	_payload->set(_key, value);
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(const char* value)
{
	//std::cout << "operator=(const char*)" << std::endl;
	_payload->set(_key, value);
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(double value)
{
	//std::cout << "operator=(double)" << std::endl;
	_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}
IPAACA_EXPORT PayloadEntryProxy& PayloadEntryProxy::operator=(bool value)
{
	//std::cout << "operator=(bool)" << std::endl;
	_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}

IPAACA_EXPORT PayloadEntryProxy::operator std::string()
{
	return PayloadEntryProxy::get<std::string>();
}
IPAACA_EXPORT inline PayloadEntryProxy::operator long()
{
	return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT inline PayloadEntryProxy::operator double()
{
	return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT inline PayloadEntryProxy::operator bool()
{
	return PayloadEntryProxy::get<bool>();
}
IPAACA_EXPORT inline std::string PayloadEntryProxy::to_str()
{
	return PayloadEntryProxy::get<std::string>(); 
}
IPAACA_EXPORT inline long PayloadEntryProxy::to_long()
{
	return PayloadEntryProxy::get<long>();
}
IPAACA_EXPORT inline double PayloadEntryProxy::to_float()
{
	return PayloadEntryProxy::get<double>();
}
IPAACA_EXPORT inline bool PayloadEntryProxy::to_bool()
{
	return PayloadEntryProxy::get<bool>();
}


//
// new stuff for protocol v2
//
IPAACA_HEADER_EXPORT template<> std::string PayloadEntryProxy::get<std::string>()
{
	return _payload->get(_key);
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
//}}}

// Payload//{{{

IPAACA_EXPORT void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = boost::weak_ptr<IUInterface>(iu);
}

IPAACA_EXPORT PayloadEntryProxy Payload::operator[](const std::string& key)
{
	//boost::shared_ptr<PayloadEntryProxy> p(new PayloadEntryProxy(this, key));
	return PayloadEntryProxy(this, key, get(key));
}
IPAACA_EXPORT Payload::operator std::map<std::string, std::string>()
{
	std::map<std::string, std::string> result;
	std::foreach(_document_store.begin(), _document_store.end(), [&result](auto pair) {
			result[pair.first] =  pair.second.GetString();
			});
	return result;
}

IPAACA_EXPORT void Payload::_internal_set(const std::string& k, const rapidjson::Document& v, const std::string& writer_name) {
	std::map<std::string, const rapidjson::Document&> _new;
	std::vector<std::string> _remove;
	_new[k]=v;
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_document_store[k] = v;
}
IPAACA_EXPORT void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	std::map<std::string, const rapidjson::Document&> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_store.erase(k);
}
IPAACA_EXPORT void Payload::_internal_replace_all(const std::map<std::string, const rapidjson::Document&>& new_contents, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(false, new_contents, _remove, writer_name );
	_store = new_contents;
}
IPAACA_EXPORT void Payload::_internal_merge(const std::map<std::string, const rapidjson::Document&>& contents_to_merge, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(true, contents_to_merge, _remove, writer_name );
	_document_store.insert(contents_to_merge.begin(), contents_to_merge.end());
	//for (std::map<std::string, std::string>::iterator it = contents_to_merge.begin(); it!=contents_to_merge.end(); i++) {
	//	_store[it->first] = it->second;
	//}
}
IPAACA_EXPORT inline PayloadDocumentEntry::ptr Payload::get(const std::string& k) {
	if (_document_store.count(k)>0) return _document_store[k];
	else return make_shared<PayloadDocumentEntry>();  // contains Document with 'null' value
}
IPAACA_EXPORT void Payload::_remotely_enforced_wipe()
{
	_document_store.clear();
}
IPAACA_EXPORT void Payload::_remotely_enforced_delitem(const std::string& k)
{
	_document_store.erase(k);
}
IPAACA_EXPORT void Payload::_remotely_enforced_setitem(const std::string& k, rapidjson::Document&& received_json_doc)
{
	_document_store[k] = make_shared<PayloadDocumentEntry>(std::move(received_json_doc));
}

//}}}

} // of namespace ipaaca
