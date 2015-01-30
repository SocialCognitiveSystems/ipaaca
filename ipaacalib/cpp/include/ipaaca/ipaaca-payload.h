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



IPAACA_HEADER_EXPORT class PayloadEntryProxy//{{{
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
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> _store;
		IPAACA_MEMBER_VAR_EXPORT boost::weak_ptr<IUInterface> _iu;
	protected:
		IPAACA_HEADER_EXPORT void initialize(boost::shared_ptr<IUInterface> iu);
		IPAACA_HEADER_EXPORT inline void _set_owner_name(const std::string& name) { _owner_name = name; }
		IPAACA_HEADER_EXPORT void _remotely_enforced_wipe();
		IPAACA_HEADER_EXPORT void _remotely_enforced_delitem(const std::string& k);
		IPAACA_HEADER_EXPORT void _remotely_enforced_setitem(const std::string& k, const std::string& v);
		IPAACA_HEADER_EXPORT void _internal_replace_all(const std::map<std::string, std::string>& new_contents, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_merge(const std::map<std::string, std::string>& contents_to_merge, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_set(const std::string& k, const std::string& v, const std::string& writer_name="");
		IPAACA_HEADER_EXPORT void _internal_remove(const std::string& k, const std::string& writer_name="");
	public:
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() { return _owner_name; }
		// access
		IPAACA_HEADER_EXPORT PayloadEntryProxy operator[](const std::string& key);
		IPAACA_HEADER_EXPORT operator std::map<std::string, std::string>();
		IPAACA_HEADER_EXPORT inline void set(const std::map<std::string, std::string>& all_elems) { _internal_replace_all(all_elems); }
		IPAACA_HEADER_EXPORT inline void set(const std::string& k, const std::string& v) { _internal_set(k, v); }
		IPAACA_HEADER_EXPORT inline void merge(const std::map<std::string, std::string>& elems_to_merge) { _internal_merge(elems_to_merge); }
		IPAACA_HEADER_EXPORT inline void remove(const std::string& k) { _internal_remove(k); }
		IPAACA_HEADER_EXPORT std::string get(const std::string& k);
	typedef boost::shared_ptr<Payload> ptr;
};//}}}

#endif
