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
	for (std::map<std::string, std::string>::const_iterator it=obj._store.begin(); it!=obj._store.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "':'" << it->second << "'";
	}
	os << "}";
	return os;
}
//}}}

// PayloadEntryProxy//{{{

IPAACA_EXPORT PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key)
: _payload(payload), _key(key)
{
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
	return PayloadEntryProxy(this, key);
}
IPAACA_EXPORT Payload::operator std::map<std::string, std::string>()
{
	return _store;
}

IPAACA_EXPORT void Payload::_internal_set(const std::string& k, const std::string& v, const std::string& writer_name) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_new[k]=v;
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_store[k] = v;
}
IPAACA_EXPORT void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu.lock()->_modify_payload(true, _new, _remove, writer_name );
	_store.erase(k);
}
IPAACA_EXPORT void Payload::_internal_replace_all(const std::map<std::string, std::string>& new_contents, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(false, new_contents, _remove, writer_name );
	_store = new_contents;
}
IPAACA_EXPORT void Payload::_internal_merge(const std::map<std::string, std::string>& contents_to_merge, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu.lock()->_modify_payload(true, contents_to_merge, _remove, writer_name );
	_store.insert(contents_to_merge.begin(), contents_to_merge.end());
	//for (std::map<std::string, std::string>::iterator it = contents_to_merge.begin(); it!=contents_to_merge.end(); i++) {
	//	_store[it->first] = it->second;
	//}
}
IPAACA_EXPORT inline std::string Payload::get(const std::string& k) {
	if (_store.count(k)>0) return _store[k];
	else return IPAACA_PAYLOAD_DEFAULT_STRING_VALUE;
}
IPAACA_EXPORT void Payload::_remotely_enforced_wipe()
{
	_store.clear();
}
IPAACA_EXPORT void Payload::_remotely_enforced_delitem(const std::string& k)
{
	_store.erase(k);
}
IPAACA_EXPORT void Payload::_remotely_enforced_setitem(const std::string& k, const std::string& v)
{
	_store[k] = v;
}

//}}}

} // of namespace ipaaca
