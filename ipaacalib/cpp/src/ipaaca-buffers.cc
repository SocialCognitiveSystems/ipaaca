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

#define VERBOSE_HANDLERS 0  // remove later

namespace ipaaca {

using namespace rsb;
using namespace rsb::filter;
using namespace rsb::converter;
using namespace rsb::patterns;

IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj)//{{{
{
	os << "PayloadUpdate(uid=" << obj.uid << ", revision=" << obj.revision;
	os << ", writer_name=" << obj.writer_name << ", is_delta=" << (obj.is_delta?"True":"False");
	os << ", new_items = {";
	bool first = true;
	for (auto& newit: obj.new_items) {
		if (first) { first=false; } else { os << ", "; }
		//os << "'" << it->first << "':'" << it->second << "'";
		os << "'" << newit.first << "': " << newit.second;
	}
	os << "}, keys_to_remove = [";
	first = true;
	for (std::vector<std::string>::const_iterator it=obj.keys_to_remove.begin(); it!=obj.keys_to_remove.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << *it << "'";
	}
	os << "])";
	return os;
}
//}}}
IPAACA_EXPORT std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj)//{{{
{
	os << "LinkUpdate(uid=" << obj.uid << ", revision=" << obj.revision;
	os << ", writer_name=" << obj.writer_name << ", is_delta=" << (obj.is_delta?"True":"False");
	os << ", new_links = {";
	bool first = true;
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj.new_links.begin(); it!=obj.new_links.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "': [";
		bool ffirst = true;
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			if (ffirst) { ffirst=false; } else { os << ", "; }
			os << "'" << *it2 << "'";
		}
		os << "]";
	}
	os << "}, links_to_remove = {";
	first = true;
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj.links_to_remove.begin(); it!=obj.links_to_remove.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "': [";
		bool ffirst = true;
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			if (ffirst) { ffirst=false; } else { os << ", "; }
			os << "'" << *it2 << "'";
		}
		os << "]";
	}
	os << "})";
	return os;
}
//}}}

// IUEventHandler//{{{
IPAACA_EXPORT IUEventHandler::IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category)
: _function(function), _event_mask(event_mask), _for_all_categories(false)
{
	if (category=="") {
		_for_all_categories = true;
	} else {
		_categories.insert(category);
	}
}
IPAACA_EXPORT IUEventHandler::IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories)
: _function(function), _event_mask(event_mask), _for_all_categories(false)
{
	if (categories.size()==0) {
		_for_all_categories = true;
	} else {
		_categories = categories;
	}
}
IPAACA_EXPORT void IUEventHandler::call(Buffer* buffer, boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category)
{
	if (_condition_met(event_type, category)) {
		//IUInterface::ptr iu = buffer->get(uid);
		//if (iu) {
#if VERBOSE_HANDLERS == 1
			std::cout << "[" << pthread_self() << " handler ENTER]" << std::endl;
#endif
			_function(iu, event_type, local);
#if VERBOSE_HANDLERS == 1
			std::cout << "[" << pthread_self() << " handler EXIT]" << std::endl;
#endif
		//}
	}
}
//}}}

// Buffer//{{{
IPAACA_EXPORT void Buffer::_allocate_unique_name(const std::string& basename, const std::string& function) {
	std::string uuid = ipaaca::generate_uuid_string();
	_basename = basename;
	_uuid = uuid.substr(0,8);
	_unique_name = "/ipaaca/component/" + _basename + "ID" + _uuid + "/" + function;
}
IPAACA_EXPORT void Buffer::register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories)
{
	std::cout << "register_handler " << function << " " << event_mask << " " << categories << std::endl;
	IUEventHandler::ptr handler = IUEventHandler::ptr(new IUEventHandler(function, event_mask, categories));
	_event_handlers.push_back(handler);
}
IPAACA_EXPORT void Buffer::register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category)
{
	std::cout << "register_handler " << function << " " << event_mask << " " << category << std::endl;
	IUEventHandler::ptr handler = IUEventHandler::ptr(new IUEventHandler(function, event_mask, category));
	_event_handlers.push_back(handler);
}
IPAACA_EXPORT void Buffer::call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category)
{
	//IPAACA_INFO("handling an event " << ipaaca::iu_event_type_to_str(event_type) << " for IU " << iu->uid())
	//std::cout << "handling an event " << ipaaca::iu_event_type_to_str(event_type) << " for IU " << iu->uid() << std::endl;
	for (std::vector<IUEventHandler::ptr>::iterator it = _event_handlers.begin(); it != _event_handlers.end(); ++it) {
		(*it)->call(this, iu, local, event_type, category);
	}
}
//}}}

// Callbacks for OutputBuffer//{{{
IPAACA_EXPORT CallbackIUPayloadUpdate::CallbackIUPayloadUpdate(Buffer* buffer): _buffer(buffer) { }
IPAACA_EXPORT CallbackIULinkUpdate::CallbackIULinkUpdate(Buffer* buffer): _buffer(buffer) { }
IPAACA_EXPORT CallbackIUCommission::CallbackIUCommission(Buffer* buffer): _buffer(buffer) { }
// dlw
IPAACA_EXPORT CallbackIUResendRequest::CallbackIUResendRequest(Buffer* buffer): _buffer(buffer) { }

IPAACA_EXPORT boost::shared_ptr<int> CallbackIUPayloadUpdate::call(const std::string& methodName, boost::shared_ptr<IUPayloadUpdate> update)
{
	IPAACA_INFO("")
	//std::cout << "-- Received a modify_payload with " << update->new_items.size() << " keys to merge." << std::endl;
	IUInterface::ptr iui = _buffer->get(update->uid);
	if (! iui) {
		IPAACA_WARNING("Remote InBuffer tried to spuriously write non-existent IU " << update->uid)
		return boost::shared_ptr<int>(new int(0));
	}
	IU::ptr iu = boost::static_pointer_cast<IU>(iui);
	iu->_revision_lock.lock();
	if ((update->revision != 0) && (update->revision != iu->_revision)) {
		IPAACA_INFO("Remote write operation failed because request was out of date; IU " << update->uid)
		IPAACA_INFO(" Referred-to revision was " << update->revision << " while local one is " << iu->_revision)
		iu->_revision_lock.unlock();
		return boost::shared_ptr<int>(new int(0));
	}
	if (update->is_delta) {
		// FIXME FIXME this is an unsolved problem atm: deletes in a delta update are
		// sent individually. We should have something like _internal_merge_and_remove
		for (std::vector<std::string>::const_iterator it=update->keys_to_remove.begin(); it!=update->keys_to_remove.end(); ++it) {
			iu->payload()._internal_remove(*it, update->writer_name); //_buffer->unique_name());
		}
		// but it is solved for pure merges:
		iu->payload()._internal_merge(update->new_items, update->writer_name);
	} else {
		iu->payload()._internal_replace_all(update->new_items, update->writer_name); //_buffer->unique_name());
	}
	//std::cout << "-- Calling update handler due to remote write." << std::endl;
	_buffer->call_iu_event_handlers(iu, true, IU_UPDATED, iu->category());
	revision_t revision = iu->revision();
	iu->_revision_lock.unlock();
	return boost::shared_ptr<int>(new int(revision));
}

IPAACA_EXPORT boost::shared_ptr<int> CallbackIULinkUpdate::call(const std::string& methodName, boost::shared_ptr<IULinkUpdate> update)
{
	IUInterface::ptr iui = _buffer->get(update->uid);
	if (! iui) {
		IPAACA_WARNING("Remote InBuffer tried to spuriously write non-existent IU " << update->uid)
		return boost::shared_ptr<int>(new int(0));
	}
	IU::ptr iu = boost::static_pointer_cast<IU>(iui);
	iu->_revision_lock.lock();
	if ((update->revision != 0) && (update->revision != iu->_revision)) {
		IPAACA_INFO("Remote write operation failed because request was out of date; IU " << update->uid)
		iu->_revision_lock.unlock();
		return boost::shared_ptr<int>(new int(0));
	}
	if (update->is_delta) {
		iu->modify_links(update->new_links, update->links_to_remove, update->writer_name);
	} else {
		iu->set_links(update->new_links, update->writer_name);
	}
	_buffer->call_iu_event_handlers(iu, true, IU_LINKSUPDATED, iu->category());
	revision_t revision = iu->revision();
	iu->_revision_lock.unlock();
	return boost::shared_ptr<int>(new int(revision));
}
IPAACA_EXPORT boost::shared_ptr<int> CallbackIUCommission::call(const std::string& methodName, boost::shared_ptr<protobuf::IUCommission> update)
{
	IUInterface::ptr iui = _buffer->get(update->uid());
	if (! iui) {
		IPAACA_WARNING("Remote InBuffer tried to spuriously write non-existent IU " << update->uid())
		return boost::shared_ptr<int>(new int(0));
	}
	IU::ptr iu = boost::static_pointer_cast<IU>(iui);
	iu->_revision_lock.lock();
	if ((update->revision() != 0) && (update->revision() != iu->_revision)) {
		IPAACA_INFO("Remote write operation failed because request was out of date; IU " << update->uid())
		iu->_revision_lock.unlock();
		return boost::shared_ptr<int>(new int(0));
	}
	if (iu->committed()) {
		return boost::shared_ptr<int>(new int(0));
	} else {
	}
	iu->_internal_commit(update->writer_name());
	_buffer->call_iu_event_handlers(iu, true, IU_LINKSUPDATED, iu->category());
	revision_t revision = iu->revision();
	iu->_revision_lock.unlock();
	return boost::shared_ptr<int>(new int(revision));
}
/** dlw */
IPAACA_EXPORT boost::shared_ptr<int> CallbackIUResendRequest::call(const std::string& methodName, boost::shared_ptr<protobuf::IUResendRequest> update)
{
	IUInterface::ptr iui = _buffer->get(update->uid());
	if (! iui) {
		IPAACA_WARNING("Remote InBuffer tried to spuriously write non-existent IU " << update->uid())
		return boost::shared_ptr<int>(new int(0));
	}
	IU::ptr iu = boost::static_pointer_cast<IU>(iui);
	if ((update->has_hidden_scope_name() == true)&&(update->hidden_scope_name().compare("") != 0)){
		//_buffer->call_iu_event_handlers(iu, true, IU_UPDATED, update->hidden_scope_name());
		revision_t revision = iu->revision();

		_buffer->_publish_iu_resend(iu, update->hidden_scope_name());
		//iu->_publish_resend(iu, update->hidden_scope_name());

		return boost::shared_ptr<int>(new int(revision));
	} else {
		revision_t revision = 0;
		return boost::shared_ptr<int>(new int(revision));
	}
}
//}}}

// OutputBuffer//{{{

IPAACA_EXPORT OutputBuffer::OutputBuffer(const std::string& basename, const std::string& channel)
:Buffer(basename, "OB")
{
	//IPAACA_INFO("Entering ...")
	_id_prefix = _basename + "-" + _uuid + "-IU-";
	_channel = (channel=="") ? __ipaaca_static_option_default_channel: channel;
	_initialize_server();
	//IPAACA_INFO("... exiting.")
}
IPAACA_EXPORT void OutputBuffer::_initialize_server()
{
	//IPAACA_INFO("Entering ...")
	_server = getFactory().createLocalServer( Scope( _unique_name ) );
	//_server = getFactory().createServer( Scope( _unique_name ) );
	//IPAACA_INFO("Registering methods")
	_server->registerMethod("updatePayload", Server::CallbackPtr(new CallbackIUPayloadUpdate(this)));
	_server->registerMethod("updateLinks", Server::CallbackPtr(new CallbackIULinkUpdate(this)));
	_server->registerMethod("commit", Server::CallbackPtr(new CallbackIUCommission(this)));
	// dlw
	_server->registerMethod("resendRequest", Server::CallbackPtr(new CallbackIUResendRequest(this)));

	//IPAACA_INFO("... exiting.")
}
IPAACA_EXPORT OutputBuffer::ptr OutputBuffer::create(const std::string& basename)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return OutputBuffer::ptr(new OutputBuffer(basename));
}
IPAACA_EXPORT IUInterface::ptr OutputBuffer::get(const std::string& iu_uid)
{
	IUStore::iterator it = _iu_store.find(iu_uid);
	if (it==_iu_store.end()) return IUInterface::ptr();
	return it->second;
}
IPAACA_EXPORT std::set<IUInterface::ptr> OutputBuffer::get_ius()
{
	std::set<IUInterface::ptr> set;
	for (IUStore::iterator it=_iu_store.begin(); it!=_iu_store.end(); ++it) set.insert(it->second);
	return set;
}

IPAACA_EXPORT void OutputBuffer::_send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	IULinkUpdate* lup = new ipaaca::IULinkUpdate();
	Informer<ipaaca::IULinkUpdate>::DataPtr ldata(lup);
	lup->uid = iu->uid();
	lup->is_delta = is_delta;
	lup->revision = revision;
	lup->is_delta = true;
	lup->new_links = new_links;
	if (is_delta) lup->links_to_remove = links_to_remove;
	if (writer_name=="") lup->writer_name = _unique_name;
	else lup->writer_name = writer_name;
	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(ldata);
}

IPAACA_EXPORT void OutputBuffer::_send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	IUPayloadUpdate* pup = new ipaaca::IUPayloadUpdate();
	Informer<ipaaca::IUPayloadUpdate>::DataPtr pdata(pup);
	pup->uid = iu->uid();
	pup->is_delta = is_delta;
	pup->revision = revision;
	pup->new_items = new_items;
	if (is_delta) pup->keys_to_remove = keys_to_remove;
	if (writer_name=="") pup->writer_name = _unique_name;
	else pup->writer_name = writer_name;
	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(pdata);
}

IPAACA_EXPORT void OutputBuffer::_send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name)
{
	Informer<protobuf::IUCommission>::DataPtr data(new protobuf::IUCommission());
	data->set_uid(iu->uid());
	data->set_revision(revision);
	if (writer_name=="") data->set_writer_name(_unique_name);
	else data->set_writer_name(writer_name);

	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(data);
}






IPAACA_EXPORT void OutputBuffer::add(IU::ptr iu)
{
	if (_iu_store.count(iu->uid()) > 0) {
		throw IUPublishedError();
	}
	if (iu->is_published()) {
		throw IUPublishedError();
	}
	if (iu->access_mode() != IU_ACCESS_MESSAGE) {
		// (for Message-type IUs: do not actually store them)
		_iu_store[iu->uid()] = iu;
	}
	iu->_associate_with_buffer(this); //shared_from_this());
	_publish_iu(iu);
}

IPAACA_EXPORT void OutputBuffer::_publish_iu(IU::ptr iu)
{
	Informer<AnyType>::Ptr informer = _get_informer(iu->_category);
	Informer<ipaaca::IU>::DataPtr iu_data(iu);
	informer->publish(iu_data);
}

IPAACA_EXPORT void OutputBuffer::_publish_iu_resend(IU::ptr iu, const std::string& hidden_scope_name)
{
	Informer<AnyType>::Ptr informer = _get_informer(hidden_scope_name);
	Informer<ipaaca::IU>::DataPtr iu_data(iu);
	informer->publish(iu_data);
}




IPAACA_EXPORT Informer<AnyType>::Ptr OutputBuffer::_get_informer(const std::string& category)
{
	if (_informer_store.count(category) > 0) {
		return _informer_store[category];
	} else {
		//IPAACA_INFO("Making new informer for category " << category)
		std::string scope_string = "/ipaaca/channel/" + _channel + "/category/" + category;

		Informer<AnyType>::Ptr informer = getFactory().createInformer<AnyType> ( Scope(scope_string));
		_informer_store[category] = informer;
		return informer;
	}
}
IPAACA_EXPORT boost::shared_ptr<IU> OutputBuffer::remove(const std::string& iu_uid)
{
	IUStore::iterator it = _iu_store.find(iu_uid);
	if (it == _iu_store.end()) {
		IPAACA_WARNING("Removal of IU " << iu_uid << " requested, but not present in our OutputBuffer")
		//throw IUNotFoundError();
	}
	IU::ptr iu = it->second;
	_retract_iu(iu);
	_iu_store.erase(iu_uid);
	return iu;
}
IPAACA_EXPORT boost::shared_ptr<IU> OutputBuffer::remove(IU::ptr iu)
{
	return remove(iu->uid()); // to make sure it is in the store
}

IPAACA_EXPORT void OutputBuffer::_retract_iu(IU::ptr iu)
{
	Informer<protobuf::IURetraction>::DataPtr data(new protobuf::IURetraction());
	data->set_uid(iu->uid());
	data->set_revision(iu->revision());
	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(data);
}


//}}}

// InputBuffer//{{{
IPAACA_EXPORT InputBuffer::InputBuffer(const BufferConfiguration& bufferconfiguration)
:Buffer(bufferconfiguration.get_basename(), "IB")
{
	_channel = bufferconfiguration.get_channel();

	for (std::vector<std::string>::const_iterator it=bufferconfiguration.get_category_interests().begin(); it!=bufferconfiguration.get_category_interests().end(); ++it) {
		_create_category_listener_if_needed(*it);
	}
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::set<std::string>& category_interests)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	for (std::set<std::string>::const_iterator it=category_interests.begin(); it!=category_interests.end(); ++it) {
		_create_category_listener_if_needed(*it);
	}
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	for (std::vector<std::string>::const_iterator it=category_interests.begin(); it!=category_interests.end(); ++it) {
		_create_category_listener_if_needed(*it);
	}
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
	_create_category_listener_if_needed(category_interest3);
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}
IPAACA_EXPORT InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4)
:Buffer(basename, "IB")
{
	_channel = __ipaaca_static_option_default_channel;

	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
	_create_category_listener_if_needed(category_interest3);
	_create_category_listener_if_needed(category_interest4);
	_create_category_listener_if_needed(_uuid);
	triggerResend = false;
}

IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const BufferConfiguration& bufferconfiguration)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(bufferconfiguration));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::set<std::string>& category_interests)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interests));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::vector<std::string>& category_interests)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interests));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2, category_interest3));
}
IPAACA_EXPORT InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2, category_interest3, category_interest4));
}


IPAACA_EXPORT void InputBuffer::set_resend(bool resendActive)
{
	triggerResend = resendActive;
}

IPAACA_EXPORT bool InputBuffer::get_resend()
{
	return triggerResend;
}


IPAACA_EXPORT IUInterface::ptr InputBuffer::get(const std::string& iu_uid)
{
	RemotePushIUStore::iterator it = _iu_store.find(iu_uid); // TODO genericize
	if (it==_iu_store.end()) return IUInterface::ptr();
	return it->second;
}
IPAACA_EXPORT std::set<IUInterface::ptr> InputBuffer::get_ius()
{
	std::set<IUInterface::ptr> set;
	for (RemotePushIUStore::iterator it=_iu_store.begin(); it!=_iu_store.end(); ++it) set.insert(it->second); // TODO genericize
	return set;
}


IPAACA_EXPORT RemoteServerPtr InputBuffer::_get_remote_server(const std::string& unique_server_name)
{
	std::map<std::string, RemoteServerPtr>::iterator it = _remote_server_store.find(unique_server_name);
	if (it!=_remote_server_store.end()) return it->second;
	RemoteServerPtr remote_server = getFactory().createRemoteServer(Scope(unique_server_name));
	_remote_server_store[unique_server_name] = remote_server;
	return remote_server;
}

IPAACA_EXPORT ListenerPtr InputBuffer::_create_category_listener_if_needed(const std::string& category)
{
	IPAACA_INFO("Entering ...")
	std::map<std::string, ListenerPtr>::iterator it = _listener_store.find(category);
	if (it!=_listener_store.end()) {
		IPAACA_INFO("... exiting.")
		return it->second;
	}
	//IPAACA_INFO("Creating a new listener for category " << category)
	std::string scope_string = "/ipaaca/channel/" + _channel + "/category/" + category;

	ListenerPtr listener = getFactory().createListener( Scope(scope_string) );
	IPAACA_INFO("Adding handler")
	HandlerPtr event_handler = HandlerPtr(
			new EventFunctionHandler(
				boost::bind(&InputBuffer::_handle_iu_events, this, _1)
			)
		);
	listener->addHandler(event_handler);
	_listener_store[category] = listener;
	IPAACA_INFO("... exiting.")
	return listener;
}
IPAACA_EXPORT void InputBuffer::_trigger_resend_request(EventPtr event) {
	if (!triggerResend) return;
	std::string type = event->getType();
	std::string uid = "";
	std::string writerName = "";
	if (type == "ipaaca::IUPayloadUpdate") {
		boost::shared_ptr<IUPayloadUpdate> update = boost::static_pointer_cast<IUPayloadUpdate>(event->getData());
		uid = update->uid;
		writerName = update->writer_name;
	} else if (type == "ipaaca::IULinkUpdate") {
		boost::shared_ptr<IULinkUpdate> update = boost::static_pointer_cast<IULinkUpdate>(event->getData());
		uid = update->uid;
		writerName = update->writer_name;
	} else if (type == "ipaaca::protobuf::IUCommission") {
		boost::shared_ptr<protobuf::IUCommission> update = boost::static_pointer_cast<protobuf::IUCommission>(event->getData());
		uid = update->uid();
		writerName = update->writer_name();
	} else {
		std::cout << "_trigger_resend_request: unhandled event type " << type << std::endl;
	}

	if (!writerName.empty()) {
		RemoteServerPtr server = _get_remote_server(writerName);
		if (!uid.empty()) {
			boost::shared_ptr<protobuf::IUResendRequest> update = boost::shared_ptr<protobuf::IUResendRequest>(new protobuf::IUResendRequest());
			update->set_uid(uid);
			update->set_hidden_scope_name(_uuid);
			boost::shared_ptr<int> result = server->call<int>("resendRequest", update, IPAACA_REMOTE_SERVER_TIMEOUT);
			if (*result == 0) {
				throw IUResendRequestFailedError();
			} else {
				//std::cout << "revision " << *result << std::endl;
			}
		}
	}
}
IPAACA_EXPORT void InputBuffer::_handle_iu_events(EventPtr event)
{
	std::cout << "handle iu events" << std::endl;
	std::string type = event->getType();
	if (type == "ipaaca::RemotePushIU") {
		boost::shared_ptr<RemotePushIU> iu = boost::static_pointer_cast<RemotePushIU>(event->getData());
		if (_iu_store.count(iu->category()) > 0) {
			// already got the IU... ignore
		} else {
			_iu_store[iu->uid()] = iu;
			iu->_set_buffer(this);
			call_iu_event_handlers(iu, false, IU_ADDED, iu->category() );
		}
		//IPAACA_INFO( "New RemotePushIU state: " << (*iu) )
	} else if (type == "ipaaca::RemoteMessage") {
		boost::shared_ptr<RemoteMessage> iu = boost::static_pointer_cast<RemoteMessage>(event->getData());
		//_iu_store[iu->uid()] = iu;
		//iu->_set_buffer(this);
		//std::cout << "REFCNT after cast, before calling handlers: " << iu.use_count() << std::endl;
		call_iu_event_handlers(iu, false, IU_MESSAGE, iu->category() );
		//_iu_store.erase(iu->uid());
	} else {
		RemotePushIUStore::iterator it;
		if (type == "ipaaca::IUPayloadUpdate") {
			boost::shared_ptr<IUPayloadUpdate> update = boost::static_pointer_cast<IUPayloadUpdate>(event->getData());
			//IPAACA_INFO("** writer name: " << update->writer_name)
			std::cout << "writer name " << update->writer_name << std::endl;
			if (update->writer_name == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid);
			if (it == _iu_store.end()) {
				_trigger_resend_request(event);
				IPAACA_INFO("Using UPDATED message for an IU that we did not fully receive before")
				return;
			}

			it->second->_apply_update(update);
			call_iu_event_handlers(it->second, false, IU_UPDATED, it->second->category() );

		} else if (type == "ipaaca::IULinkUpdate") {
			boost::shared_ptr<IULinkUpdate> update = boost::static_pointer_cast<IULinkUpdate>(event->getData());
			if (update->writer_name == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid);
			if (it == _iu_store.end()) {
				_trigger_resend_request(event);
				IPAACA_INFO("Ignoring LINKSUPDATED message for an IU that we did not fully receive before")
				return;
			}

			it->second->_apply_link_update(update);
			call_iu_event_handlers(it->second, false, IU_LINKSUPDATED, it->second->category() );

		} else if (type == "ipaaca::protobuf::IUCommission") {
			boost::shared_ptr<protobuf::IUCommission> update = boost::static_pointer_cast<protobuf::IUCommission>(event->getData());
			if (update->writer_name() == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid());
			if (it == _iu_store.end()) {
				_trigger_resend_request(event);
				IPAACA_INFO("Ignoring COMMITTED message for an IU that we did not fully receive before")
				return;
			}
			//
			it->second->_apply_commission();
			it->second->_revision = update->revision();
			call_iu_event_handlers(it->second, false, IU_COMMITTED, it->second->category() );
			//
			//
		} else if (type == "ipaaca::protobuf::IURetraction") {
			boost::shared_ptr<protobuf::IURetraction> update = boost::static_pointer_cast<protobuf::IURetraction>(event->getData());
			it = _iu_store.find(update->uid());
			if (it == _iu_store.end()) {
				_trigger_resend_request(event);
				IPAACA_INFO("Ignoring RETRACTED message for an IU that we did not fully receive before")
				return;
			}
			//
			it->second->_revision = update->revision();
			it->second->_apply_retraction();
			// remove from InputBuffer     FIXME: this is a crossover between retracted and deleted behavior
			_iu_store.erase(it->first);
			// and call the handler. IU reference is still valid for this call, although removed from buffer.
			call_iu_event_handlers(it->second, false, IU_COMMITTED, it->second->category() );
			//
		} else {
			std::cout << "(Unhandled Event type " << type << " !)" << std::endl;
			return;
		}
		//IPAACA_INFO( "New RemotePushIU state: " << *(it->second) )
	}
}
//}}}

} // of namespace ipaaca
