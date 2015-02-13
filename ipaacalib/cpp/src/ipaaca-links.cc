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

IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const SmartLinkMap& obj)//{{{
{
	os << "{";
	bool first = true;
	for (LinkMap::const_iterator it=obj._links.begin(); it!=obj._links.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "': [";
		bool firstinner = true;
		for (LinkSet::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			if (firstinner) { firstinner=false; } else { os << ", "; }
			os << "'" << *it2 << "'";
		}
		os << "]";
	}
	os << "}";
	return os;
}
//}}}

// SmartLinkMap//{{{

IPAACA_EXPORT LinkSet SmartLinkMap::empty_link_set;
IPAACA_EXPORT void SmartLinkMap::_add_and_remove_links(const LinkMap& add, const LinkMap& remove)
{
	// remove specified links
	for (LinkMap::const_iterator it = remove.begin(); it != remove.end(); ++it ) {
		// if link type exists
		if (_links.count(it->first) > 0) {
			// remove one by one
			for (LinkSet::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
				_links[it->first].erase(*it2);
			}
			// wipe the type key if no more links are left
			if (_links[it->first].size() == 0) {
				_links.erase(it->first);
			}
		}
	}
	// add specified links
	for (LinkMap::const_iterator it = add.begin(); it != add.end(); ++it ) {
		for (LinkSet::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			_links[it->first].insert(*it2);
		}
	}
}
IPAACA_EXPORT void SmartLinkMap::_replace_links(const LinkMap& links)
{
	//_links.clear();
	_links=links;
}
IPAACA_EXPORT const LinkSet& SmartLinkMap::get_links(const std::string& key)
{
	LinkMap::const_iterator it = _links.find(key);
	if (it==_links.end()) return empty_link_set;
	return it->second;
}
IPAACA_EXPORT const LinkMap& SmartLinkMap::get_all_links()
{
	return _links;
}
//}}}

} // of namespace ipaaca

