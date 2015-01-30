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

// IU//{{{
IPAACA_EXPORT IU::ptr IU::create(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	IU::ptr iu = IU::ptr(new IU(category, access_mode, read_only, payload_type)); /* params */ //));
	iu->_payload.initialize(iu);
	return iu;
}

IPAACA_EXPORT IU::IU(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	_revision = 1;
	_uid = ipaaca::generate_uuid_string();
	_category = category;
	_payload_type = payload_type;
	// payload initialization deferred to IU::create(), above
	_read_only = read_only;
	_access_mode = access_mode;
	_committed = false;
}

IPAACA_EXPORT void IU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	if (is_published()) {
		_buffer->_send_iu_link_update(this, is_delta, _revision, new_links, links_to_remove, writer_name);
	}
	_revision_lock.unlock();
}


IPAACA_EXPORT void IU::_publish_resend(IU::ptr iu, const std::string& hidden_scope_name)
{
	//_revision_lock.lock();
	//if (_committed) {
	//	_revision_lock.unlock();
	//	throw IUCommittedError();
	//}
	//_increase_revision_number();
	//if (is_published()) {
	//IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name
	_buffer->_publish_iu_resend(iu, hidden_scope_name);
	//}
	//_revision_lock.unlock();
}





IPAACA_EXPORT void IU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	if (is_published()) {
		//std::cout << "Sending a payload update with " << new_items.size() << " entries to merge." << std::endl;
		_buffer->_send_iu_payload_update(this, is_delta, _revision, new_items, keys_to_remove, writer_name);
	}
	_revision_lock.unlock();
}

IPAACA_EXPORT void IU::commit()
{
	_internal_commit();
}

IPAACA_EXPORT void IU::_internal_commit(const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	_committed = true;
	if (is_published()) {
		_buffer->_send_iu_commission(this, _revision, writer_name);
	}
	_revision_lock.unlock();
}
//}}}
// Message//{{{
Message::ptr Message::create(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	Message::ptr iu = Message::ptr(new Message(category, access_mode, read_only, payload_type)); /* params */ //));
	iu->_payload.initialize(iu);
	return iu;
}

Message::Message(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
: IU(category, access_mode, read_only, payload_type)
{
}

void Message::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	if (is_published()) {
		IPAACA_INFO("Info: modifying a Message after sending has no global effects")
	}
}
void Message::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	if (is_published()) {
		IPAACA_INFO("Info: modifying a Message after sending has no global effects")
	}
}

void Message::_internal_commit(const std::string& writer_name)
{
	if (is_published()) {
		IPAACA_INFO("Info: committing to a Message after sending has no global effects")
	}

}
//}}}

// RemotePushIU//{{{

IPAACA_EXPORT RemotePushIU::ptr RemotePushIU::create()
{
	RemotePushIU::ptr iu = RemotePushIU::ptr(new RemotePushIU(/* params */));
	iu->_payload.initialize(iu);
	return iu;
}
IPAACA_EXPORT RemotePushIU::RemotePushIU()
{
	// nothing
}
IPAACA_EXPORT void RemotePushIU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	if (_committed) {
		throw IUCommittedError();
	}
	if (_read_only) {
		throw IUReadOnlyError();
	}
	RemoteServerPtr server = boost::static_pointer_cast<InputBuffer>(_buffer)->_get_remote_server(_owner_name);
	IULinkUpdate::ptr update = IULinkUpdate::ptr(new IULinkUpdate());
	update->uid = _uid;
	update->revision = _revision;
	update->is_delta = is_delta;
	update->writer_name = _buffer->unique_name();
	update->new_links = new_links;
	update->links_to_remove = links_to_remove;
	boost::shared_ptr<int> result = server->call<int>("updateLinks", update, IPAACA_REMOTE_SERVER_TIMEOUT); // TODO
	if (*result == 0) {
		throw IUUpdateFailedError();
	} else {
		_revision = *result;
	}
}
IPAACA_EXPORT void RemotePushIU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	//std::cout << "-- Sending a modify_payload with " << new_items.size() << " keys to merge." << std::endl;
	if (_committed) {
		throw IUCommittedError();
	}
	if (_read_only) {
		throw IUReadOnlyError();
	}
	RemoteServerPtr server = boost::static_pointer_cast<InputBuffer>(_buffer)->_get_remote_server(_owner_name);
	IUPayloadUpdate::ptr update = IUPayloadUpdate::ptr(new IUPayloadUpdate());
	update->uid = _uid;
	update->revision = _revision;
	update->is_delta = is_delta;
	update->writer_name = _buffer->unique_name();
	update->new_items = new_items;
	update->keys_to_remove = keys_to_remove;
	boost::shared_ptr<int> result = server->call<int>("updatePayload", update, IPAACA_REMOTE_SERVER_TIMEOUT); // TODO
	if (*result == 0) {
		throw IUUpdateFailedError();
	} else {
		_revision = *result;
	}
}

IPAACA_EXPORT void RemotePushIU::commit()
{
	if (_read_only) {
		throw IUReadOnlyError();
	}
	if (_committed) {
		// Following python version: ignoring multiple commit
		return;
	}
	RemoteServerPtr server = boost::static_pointer_cast<InputBuffer>(_buffer)->_get_remote_server(_owner_name);
	boost::shared_ptr<protobuf::IUCommission> update = boost::shared_ptr<protobuf::IUCommission>(new protobuf::IUCommission());
	update->set_uid(_uid);
	update->set_revision(_revision);
	update->set_writer_name(_buffer->unique_name());
	boost::shared_ptr<int> result = server->call<int>("commit", update, IPAACA_REMOTE_SERVER_TIMEOUT); // TODO
	if (*result == 0) {
		throw IUUpdateFailedError();
	} else {
		_revision = *result;
	}
}

IPAACA_EXPORT void RemotePushIU::_apply_link_update(IULinkUpdate::ptr update)
{
	_revision = update->revision;
	if (update->is_delta) {
		_add_and_remove_links(update->new_links, update->links_to_remove);
	} else {
		_replace_links(update->new_links);
	}
}
IPAACA_EXPORT void RemotePushIU::_apply_update(IUPayloadUpdate::ptr update)
{
	_revision = update->revision;
	if (update->is_delta) {
		for (std::vector<std::string>::const_iterator it=update->keys_to_remove.begin(); it!=update->keys_to_remove.end(); ++it) {
			_payload._remotely_enforced_delitem(*it);
		}
		for (std::map<std::string, std::string>::const_iterator it=update->new_items.begin(); it!=update->new_items.end(); ++it) {
			_payload._remotely_enforced_setitem(it->first, it->second);
		}
	} else {
		_payload._remotely_enforced_wipe();
		for (std::map<std::string, std::string>::const_iterator it=update->new_items.begin(); it!=update->new_items.end(); ++it) {
			_payload._remotely_enforced_setitem(it->first, it->second);
		}
	}
}
IPAACA_EXPORT void RemotePushIU::_apply_commission()
{
	_committed = true;
}
IPAACA_EXPORT void RemotePushIU::_apply_retraction()
{
	_retracted = true;
}
//}}}

// RemoteMessage//{{{

IPAACA_EXPORT RemoteMessage::ptr RemoteMessage::create()
{
	RemoteMessage::ptr iu = RemoteMessage::ptr(new RemoteMessage(/* params */));
	iu->_payload.initialize(iu);
	return iu;
}
IPAACA_EXPORT RemoteMessage::RemoteMessage()
{
	// nothing
}
IPAACA_EXPORT void RemoteMessage::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	IPAACA_INFO("Info: modifying a RemoteMessage only has local effects")
}
IPAACA_EXPORT void RemoteMessage::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	IPAACA_INFO("Info: modifying a RemoteMessage only has local effects")
}
IPAACA_EXPORT void RemoteMessage::commit()
{
	IPAACA_INFO("Info: committing to a RemoteMessage only has local effects")
}

IPAACA_EXPORT void RemoteMessage::_apply_link_update(IULinkUpdate::ptr update)
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_link_update")
	_revision = update->revision;
	if (update->is_delta) {
		_add_and_remove_links(update->new_links, update->links_to_remove);
	} else {
		_replace_links(update->new_links);
	}
}
IPAACA_EXPORT void RemoteMessage::_apply_update(IUPayloadUpdate::ptr update)
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_update")
	_revision = update->revision;
	if (update->is_delta) {
		for (std::vector<std::string>::const_iterator it=update->keys_to_remove.begin(); it!=update->keys_to_remove.end(); ++it) {
			_payload._remotely_enforced_delitem(*it);
		}
		for (std::map<std::string, std::string>::const_iterator it=update->new_items.begin(); it!=update->new_items.end(); ++it) {
			_payload._remotely_enforced_setitem(it->first, it->second);
		}
	} else {
		_payload._remotely_enforced_wipe();
		for (std::map<std::string, std::string>::const_iterator it=update->new_items.begin(); it!=update->new_items.end(); ++it) {
			_payload._remotely_enforced_setitem(it->first, it->second);
		}
	}
}
IPAACA_EXPORT void RemoteMessage::_apply_commission()
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_commission")
	_committed = true;
}
IPAACA_EXPORT void RemoteMessage::_apply_retraction()
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_retraction")
	_retracted = true;
}

//}}}

} // of namespace ipaaca
