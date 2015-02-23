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

IPAACA_EXPORT inline FakeIU::FakeIU() {
	IPAACA_INFO("")
}
IPAACA_EXPORT boost::shared_ptr<FakeIU> FakeIU::create()
{
	IPAACA_INFO("");
	auto iu = boost::shared_ptr<FakeIU>(new FakeIU());
	iu->_payload.initialize(iu);
	return iu;
}
IPAACA_EXPORT void FakeIU::add_fake_payload_item(const std::string& key, PayloadDocumentEntry::ptr entry)
{
	_payload._remotely_enforced_setitem(key, entry);
}
IPAACA_EXPORT inline FakeIU::~FakeIU() { }
IPAACA_EXPORT inline Payload& FakeIU::payload() { return _payload; }
IPAACA_EXPORT inline const Payload& FakeIU::const_payload() const { return _payload; }
IPAACA_EXPORT inline void FakeIU::commit() { }
IPAACA_EXPORT inline void FakeIU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name) { }
IPAACA_EXPORT inline void FakeIU::_modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name) { }
IPAACA_EXPORT inline void FakeIU::_apply_update(IUPayloadUpdate::ptr update) { }
IPAACA_EXPORT inline void FakeIU::_apply_link_update(IULinkUpdate::ptr update) { }
IPAACA_EXPORT inline void FakeIU::_apply_commission() { }
IPAACA_EXPORT inline void FakeIU::_apply_retraction() { }

} // of namespace ipaaca


