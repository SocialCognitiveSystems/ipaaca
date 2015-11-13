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

IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const IUInterface& obj)//{{{
{
	os << "IUInterface(uid='" << obj.uid() << "'";
	os << ", category='" << obj.category() << "'";
	os << ", revision=" << obj.revision();
	os << ", committed=" << (obj.committed()?"True":"False");
	os << ", owner_name='" << obj.owner_name() << "'";
	os << ", payload=";
	os << obj.const_payload();
	os << ", links=";
	os << obj._links;
	os << ")";
	return os;
}
//}}}

// IUInterface//{{{

IPAACA_EXPORT IUInterface::IUInterface()
: _buffer(NULL), _committed(false), _retracted(false)
{
}

IPAACA_EXPORT void IUInterface::_set_uid(const std::string& uid) {
	if (_uid != "") {
		throw IUAlreadyHasAnUIDError();
	}
	_uid = uid;
}

IPAACA_EXPORT void IUInterface::_set_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	if (_buffer) {
		throw IUAlreadyInABufferError();
	}
	_buffer = buffer;

}

IPAACA_EXPORT void IUInterface::_set_owner_name(const std::string& owner_name) {
	if (_owner_name != "") {
		throw IUAlreadyHasAnOwnerNameError();
	}
	_owner_name = owner_name;
}

/// set the buffer pointer and the owner names of IU and Payload
IPAACA_EXPORT void IUInterface::_associate_with_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	_set_buffer(buffer); // will throw if already set
	_set_owner_name(buffer->unique_name());
	payload()._set_owner_name(buffer->unique_name());
}

/// C++-specific convenience function to add one single link
IPAACA_EXPORT void IUInterface::add_link(const std::string& type, const std::string& target, const std::string& writer_name)
{
	LinkMap none;
	LinkMap add;
	add[type].insert(target);
	_modify_links(true, add, none, writer_name);
	_add_and_remove_links(add, none);
}
/// C++-specific convenience function to remove one single link
IPAACA_EXPORT void IUInterface::remove_link(const std::string& type, const std::string& target, const std::string& writer_name)
{
	LinkMap none;
	LinkMap remove;
	remove[type].insert(target);
	_modify_links(true, none, remove, writer_name);
	_add_and_remove_links(none, remove);
}

IPAACA_EXPORT void IUInterface::add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap add;
	add[type] = targets;
	_modify_links(true, add, none, writer_name);
	_add_and_remove_links(add, none);
}

IPAACA_EXPORT void IUInterface::remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap remove;
	remove[type] = targets;
	_modify_links(true, none, remove, writer_name);
	_add_and_remove_links(none, remove);
}

IPAACA_EXPORT void IUInterface::modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name)
{
	_modify_links(true, add, remove, writer_name);
	_add_and_remove_links(add, remove);
}

IPAACA_EXPORT void IUInterface::set_links(const LinkMap& links, const std::string& writer_name)
{
	LinkMap none;
	_modify_links(false, links, none, writer_name);
	_replace_links(links);
}

IPAACA_HEADER_EXPORT const std::string& IUInterface::channel()
{
	if (_buffer == NULL)
		throw IUUnpublishedError();
	else
		return _buffer->channel();

}

//}}}

} // of namespace ipaaca
