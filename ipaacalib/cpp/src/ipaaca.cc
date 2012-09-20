#include <ipaaca/ipaaca.h>
#include <cstdlib>

namespace ipaaca {

using namespace rsb;
using namespace rsb::filter;
using namespace rsb::converter;
using namespace rsb::patterns;

// util and init//{{{

bool Initializer::_initialized = false;

//const LinkSet EMPTY_LINK_SET = LinkSet();
//const std::set<std::string> EMPTY_LINK_SET();
bool Initializer::initialized() { return _initialized; }
void Initializer::initialize_ipaaca_rsb_if_needed()
{
	if (_initialized) return;
	ParticipantConfig config = ParticipantConfig::fromConfiguration();
	Factory::getInstance().setDefaultParticipantConfig(config);
	
	boost::shared_ptr<IUConverter> iu_converter(new IUConverter());
	converterRepository<std::string>()->registerConverter(iu_converter);
	
	boost::shared_ptr<MessageConverter> message_converter(new MessageConverter());
	converterRepository<std::string>()->registerConverter(message_converter);
	
	boost::shared_ptr<IUPayloadUpdateConverter> payload_update_converter(new IUPayloadUpdateConverter());
	converterRepository<std::string>()->registerConverter(payload_update_converter);
	
	boost::shared_ptr<IULinkUpdateConverter> link_update_converter(new IULinkUpdateConverter());
	converterRepository<std::string>()->registerConverter(link_update_converter);
	
	boost::shared_ptr<ProtocolBufferConverter<protobuf::IUCommission> > iu_commission_converter(new ProtocolBufferConverter<protobuf::IUCommission> ());
	converterRepository<std::string>()->registerConverter(iu_commission_converter);
	
	boost::shared_ptr<ProtocolBufferConverter<protobuf::IURetraction> > iu_retraction_converter(new ProtocolBufferConverter<protobuf::IURetraction> ());
	converterRepository<std::string>()->registerConverter(iu_retraction_converter);
	
	boost::shared_ptr<IntConverter> int_converter(new IntConverter());
	converterRepository<std::string>()->registerConverter(int_converter);
	
	_initialized = true;
	//IPAACA_TODO("initialize all converters")
}

std::string generate_uuid_string()
{
	uuid_t uuidt;
	uuid_generate(uuidt);
#ifdef __MACOSX__
	uuid_string_t uuidstr;
	uuid_unparse_lower(uuidt, uuidstr);
	return uuidstr;
#else
	char result_c[37];
	uuid_unparse_lower(uuidt, result_c);
	return result_c;
#endif
}

/*
void init_inprocess_too() {
	//ParticipantConfig config = Factory::getInstance().getDefaultParticipantConfig();
	ParticipantConfig config = ParticipantConfig::fromFile("rsb.cfg");
	//ParticipantConfig::Transport inprocess = config.getTransport("inprocess");
	//inprocess.setEnabled(true);
	//config.addTransport(inprocess);
	Factory::getInstance().setDefaultParticipantConfig(config);
}
*/
//}}}

std::ostream& operator<<(std::ostream& os, const SmartLinkMap& obj)//{{{
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
std::ostream& operator<<(std::ostream& os, const Payload& obj)//{{{
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
std::ostream& operator<<(std::ostream& os, const IUInterface& obj)//{{{
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
std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj)//{{{
{
	os << "PayloadUpdate(uid=" << obj.uid << ", revision=" << obj.revision;
	os << ", writer_name=" << obj.writer_name << ", is_delta=" << (obj.is_delta?"True":"False");
	os << ", new_items = {";
	bool first = true;
	for (std::map<std::string, std::string>::const_iterator it=obj.new_items.begin(); it!=obj.new_items.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "':'" << it->second << "'";
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
std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj)//{{{
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

// SmartLinkMap//{{{

LinkSet SmartLinkMap::empty_link_set;
void SmartLinkMap::_add_and_remove_links(const LinkMap& add, const LinkMap& remove)
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
void SmartLinkMap::_replace_links(const LinkMap& links)
{
	//_links.clear();
	_links=links;
}
const LinkSet& SmartLinkMap::get_links(const std::string& key)
{
	LinkMap::const_iterator it = _links.find(key);
	if (it==_links.end()) return empty_link_set;
	return it->second;
}
const LinkMap& SmartLinkMap::get_all_links()
{
	return _links;
}
//}}}

// IUEventHandler//{{{
IUEventHandler::IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category)
: _function(function), _event_mask(event_mask), _for_all_categories(false)
{
	if (category=="") {
		_for_all_categories = true;
	} else {
		_categories.insert(category);
	}
}
IUEventHandler::IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories)
: _function(function), _event_mask(event_mask), _for_all_categories(false)
{
	if (categories.size()==0) {
		_for_all_categories = true;
	} else {
		_categories = categories;
	}
}
void IUEventHandler::call(Buffer* buffer, boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category)
{
	if (_condition_met(event_type, category)) {
		//IUInterface::ptr iu = buffer->get(uid);
		//if (iu) {
			_function(iu, event_type, local);
		//}
	}
}
//}}}

// Buffer//{{{
void Buffer::_allocate_unique_name(const std::string& basename, const std::string& function) {
	std::string uuid = ipaaca::generate_uuid_string();
	_basename = basename;
	_uuid = uuid.substr(0,8);
	_unique_name = "/ipaaca/component/" + _basename + "ID" + _uuid + "/" + function;
}
void Buffer::register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories)
{
	IUEventHandler::ptr handler = IUEventHandler::ptr(new IUEventHandler(function, event_mask, categories));
	_event_handlers.push_back(handler);
}
void Buffer::register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category)
{
	IUEventHandler::ptr handler = IUEventHandler::ptr(new IUEventHandler(function, event_mask, category));
	_event_handlers.push_back(handler);
}
void Buffer::call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category)
{
	//IPAACA_INFO("handling an event " << ipaaca::iu_event_type_to_str(event_type) << " for IU " << iu->uid())
	for (std::vector<IUEventHandler::ptr>::iterator it = _event_handlers.begin(); it != _event_handlers.end(); ++it) {
		(*it)->call(this, iu, local, event_type, category);
	}
}
//}}}

// Callbacks for OutputBuffer//{{{
CallbackIUPayloadUpdate::CallbackIUPayloadUpdate(Buffer* buffer): _buffer(buffer) { }
CallbackIULinkUpdate::CallbackIULinkUpdate(Buffer* buffer): _buffer(buffer) { }
CallbackIUCommission::CallbackIUCommission(Buffer* buffer): _buffer(buffer) { }

boost::shared_ptr<int> CallbackIUPayloadUpdate::call(const std::string& methodName, boost::shared_ptr<IUPayloadUpdate> update)
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
		for (std::vector<std::string>::const_iterator it=update->keys_to_remove.begin(); it!=update->keys_to_remove.end(); ++it) {
			iu->payload()._internal_remove(*it, update->writer_name); //_buffer->unique_name());
		}
		for (std::map<std::string, std::string>::const_iterator it=update->new_items.begin(); it!=update->new_items.end(); ++it) {
			iu->payload()._internal_set(it->first, it->second, update->writer_name); //_buffer->unique_name());
		}
	} else {
		iu->payload()._internal_replace_all(update->new_items, update->writer_name); //_buffer->unique_name());
	}
	_buffer->call_iu_event_handlers(iu, true, IU_UPDATED, iu->category());
	revision_t revision = iu->revision();
	iu->_revision_lock.unlock();
	return boost::shared_ptr<int>(new int(revision));
}

boost::shared_ptr<int> CallbackIULinkUpdate::call(const std::string& methodName, boost::shared_ptr<IULinkUpdate> update)
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
boost::shared_ptr<int> CallbackIUCommission::call(const std::string& methodName, boost::shared_ptr<protobuf::IUCommission> update)
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

//}}}

// OutputBuffer//{{{

OutputBuffer::OutputBuffer(const std::string& basename)
:Buffer(basename, "OB")
{
	_id_prefix = _basename + "-" + _uuid + "-IU-";
	_initialize_server();
}
void OutputBuffer::_initialize_server()
{
	_server = Factory::getInstance().createServer( Scope( _unique_name ) );
	_server->registerMethod("updatePayload", Server::CallbackPtr(new CallbackIUPayloadUpdate(this)));
	_server->registerMethod("updateLinks", Server::CallbackPtr(new CallbackIULinkUpdate(this)));
	_server->registerMethod("commit", Server::CallbackPtr(new CallbackIUCommission(this)));
}
OutputBuffer::ptr OutputBuffer::create(const std::string& basename)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return OutputBuffer::ptr(new OutputBuffer(basename));
}
IUInterface::ptr OutputBuffer::get(const std::string& iu_uid)
{
	IUStore::iterator it = _iu_store.find(iu_uid);
	if (it==_iu_store.end()) return IUInterface::ptr();
	return it->second;
}
std::set<IUInterface::ptr> OutputBuffer::get_ius()
{
	std::set<IUInterface::ptr> set;
	for (IUStore::iterator it=_iu_store.begin(); it!=_iu_store.end(); ++it) set.insert(it->second);
	return set;
}

void OutputBuffer::_send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
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

void OutputBuffer::_send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
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

void OutputBuffer::_send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name)
{
	Informer<protobuf::IUCommission>::DataPtr data(new protobuf::IUCommission());
	data->set_uid(iu->uid());
	data->set_revision(revision);
	if (writer_name=="") data->set_writer_name(_unique_name);
	else data->set_writer_name(writer_name);
	
	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(data);
}

void OutputBuffer::add(IU::ptr iu)
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

void OutputBuffer::_publish_iu(IU::ptr iu)
{
	Informer<AnyType>::Ptr informer = _get_informer(iu->_category);
	Informer<ipaaca::IU>::DataPtr iu_data(iu);
	informer->publish(iu_data);
}

Informer<AnyType>::Ptr OutputBuffer::_get_informer(const std::string& category)
{
	if (_informer_store.count(category) > 0) {
		return _informer_store[category];
	} else {
		//IPAACA_INFO("Making new informer for category " << category)
		std::string scope_string = "/ipaaca/category/" + category;
		Informer<AnyType>::Ptr informer = Factory::getInstance().createInformer<AnyType> ( Scope(scope_string));
		_informer_store[category] = informer;
		return informer;
	}
}
boost::shared_ptr<IU> OutputBuffer::remove(const std::string& iu_uid)
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
boost::shared_ptr<IU> OutputBuffer::remove(IU::ptr iu)
{
	return remove(iu->uid()); // to make sure it is in the store
}

void OutputBuffer::_retract_iu(IU::ptr iu)
{
	Informer<protobuf::IURetraction>::DataPtr data(new protobuf::IURetraction());
	data->set_uid(iu->uid());
	data->set_revision(iu->revision());
	Informer<AnyType>::Ptr informer = _get_informer(iu->category());
	informer->publish(data);
}


//}}}

// InputBuffer//{{{
InputBuffer::InputBuffer(const std::string& basename, const std::set<std::string>& category_interests)
:Buffer(basename, "IB")
{
	for (std::set<std::string>::const_iterator it=category_interests.begin(); it!=category_interests.end(); ++it) {
		_create_category_listener_if_needed(*it);
	}
}
InputBuffer::InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests)
:Buffer(basename, "IB")
{
	for (std::vector<std::string>::const_iterator it=category_interests.begin(); it!=category_interests.end(); ++it) {
		_create_category_listener_if_needed(*it);
	}
}
InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1)
:Buffer(basename, "IB")
{
	_create_category_listener_if_needed(category_interest1);
}
InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2)
:Buffer(basename, "IB")
{
	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
}
InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3)
:Buffer(basename, "IB")
{
	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
	_create_category_listener_if_needed(category_interest3);
}
InputBuffer::InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4)
:Buffer(basename, "IB")
{
	_create_category_listener_if_needed(category_interest1);
	_create_category_listener_if_needed(category_interest2);
	_create_category_listener_if_needed(category_interest3);
	_create_category_listener_if_needed(category_interest4);
}


InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::set<std::string>& category_interests)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interests));
}
InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::vector<std::string>& category_interests)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interests));
}
InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1));
}
InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2));
}
InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2, category_interest3));
}
InputBuffer::ptr InputBuffer::create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4)
{
	Initializer::initialize_ipaaca_rsb_if_needed();
	return InputBuffer::ptr(new InputBuffer(basename, category_interest1, category_interest2, category_interest3, category_interest4));
}

IUInterface::ptr InputBuffer::get(const std::string& iu_uid)
{
	RemotePushIUStore::iterator it = _iu_store.find(iu_uid); // TODO genericize
	if (it==_iu_store.end()) return IUInterface::ptr();
	return it->second;
}
std::set<IUInterface::ptr> InputBuffer::get_ius()
{
	std::set<IUInterface::ptr> set;
	for (RemotePushIUStore::iterator it=_iu_store.begin(); it!=_iu_store.end(); ++it) set.insert(it->second); // TODO genericize
	return set;
}


RemoteServerPtr InputBuffer::_get_remote_server(const std::string& unique_server_name)
{
	std::map<std::string, RemoteServerPtr>::iterator it = _remote_server_store.find(unique_server_name);
	if (it!=_remote_server_store.end()) return it->second;
	RemoteServerPtr remote_server = Factory::getInstance().createRemoteServer(Scope(unique_server_name));
	_remote_server_store[unique_server_name] = remote_server;
	return remote_server;
}

ListenerPtr InputBuffer::_create_category_listener_if_needed(const std::string& category)
{
	std::map<std::string, ListenerPtr>::iterator it = _listener_store.find(category);
	if (it!=_listener_store.end()) return it->second;
	//IPAACA_INFO("Creating a new listener for category " << category)
	std::string scope_string = "/ipaaca/category/" + category;
	ListenerPtr listener = Factory::getInstance().createListener( Scope(scope_string) );
	HandlerPtr event_handler = HandlerPtr(
			new EventFunctionHandler(
				boost::bind(&InputBuffer::_handle_iu_events, this, _1)
			)
		);
	listener->addHandler(event_handler);
	_listener_store[category] = listener;
	return listener;
	/*
		'''Return (or create, store and return) a category listener.'''
		if iu_category in self._listener_store: return self._informer_store[iu_category]
		cat_listener = rsb.createListener(rsb.Scope("/ipaaca/category/"+str(iu_category)), config=self._participant_config)
		cat_listener.addHandler(self._handle_iu_events)
		self._listener_store[iu_category] = cat_listener
		self._category_interests.append(iu_category)
		logger.info("Added listener in scope "+"/ipaaca/category/"+iu_category)
		return cat_listener
	*/
}
void InputBuffer::_handle_iu_events(EventPtr event)
{
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
		call_iu_event_handlers(iu, false, IU_MESSAGE, iu->category() );
		//_iu_store.erase(iu->uid());
	} else {
		RemotePushIUStore::iterator it;
		if (type == "ipaaca::IUPayloadUpdate") {
			boost::shared_ptr<IUPayloadUpdate> update = boost::static_pointer_cast<IUPayloadUpdate>(event->getData());
			//IPAACA_INFO("** writer name: " << update->writer_name)
			if (update->writer_name == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid);
			if (it == _iu_store.end()) {
				IPAACA_INFO("Ignoring UPDATED message for an IU that we did not fully receive before")
				return;
			}
			//
			it->second->_apply_update(update);
			call_iu_event_handlers(it->second, false, IU_UPDATED, it->second->category() );
			//
			//
		} else if (type == "ipaaca::IULinkUpdate") {
			boost::shared_ptr<IULinkUpdate> update = boost::static_pointer_cast<IULinkUpdate>(event->getData());
			if (update->writer_name == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid);
			if (it == _iu_store.end()) {
				IPAACA_INFO("Ignoring LINKSUPDATED message for an IU that we did not fully receive before")
				return;
			}
			//
			it->second->_apply_link_update(update);
			call_iu_event_handlers(it->second, false, IU_LINKSUPDATED, it->second->category() );
			//
			//
		} else if (type == "ipaaca::protobuf::IUCommission") {
			boost::shared_ptr<protobuf::IUCommission> update = boost::static_pointer_cast<protobuf::IUCommission>(event->getData());
			if (update->writer_name() == _unique_name) {
				return;
			}
			it = _iu_store.find(update->uid());
			if (it == _iu_store.end()) {
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



// IUInterface//{{{

IUInterface::IUInterface()
: _buffer(NULL), _committed(false), _retracted(false)
{
}

void IUInterface::_set_uid(const std::string& uid) {
	if (_uid != "") {
		throw IUAlreadyHasAnUIDError();
	}
	_uid = uid;
}

void IUInterface::_set_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	if (_buffer) {
		throw IUAlreadyInABufferError();
	}
	_buffer = buffer;
	
}

void IUInterface::_set_owner_name(const std::string& owner_name) {
	if (_owner_name != "") {
		throw IUAlreadyHasAnOwnerNameError();
	}
	_owner_name = owner_name;
}

/// set the buffer pointer and the owner names of IU and Payload
void IUInterface::_associate_with_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	_set_buffer(buffer); // will throw if already set
	_set_owner_name(buffer->unique_name());
	payload()._set_owner_name(buffer->unique_name());
}

/// C++-specific convenience function to add one single link
void IUInterface::add_link(const std::string& type, const std::string& target, const std::string& writer_name)
{
	LinkMap none;
	LinkMap add;
	add[type].insert(target);
	_modify_links(true, add, none, writer_name);
	_add_and_remove_links(add, none);
}
/// C++-specific convenience function to remove one single link
void IUInterface::remove_link(const std::string& type, const std::string& target, const std::string& writer_name)
{
	LinkMap none;
	LinkMap remove;
	remove[type].insert(target);
	_modify_links(true, none, remove, writer_name);
	_add_and_remove_links(none, remove);
}

void IUInterface::add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap add;
	add[type] = targets;
	_modify_links(true, add, none, writer_name);
	_add_and_remove_links(add, none);
}

void IUInterface::remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap remove;
	remove[type] = targets;
	_modify_links(true, none, remove, writer_name);
	_add_and_remove_links(none, remove);
}

void IUInterface::modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name)
{
	_modify_links(true, add, remove, writer_name);
	_add_and_remove_links(add, remove);
}

void IUInterface::set_links(const LinkMap& links, const std::string& writer_name)
{
	LinkMap none;
	_modify_links(false, links, none, writer_name);
	_replace_links(links);
}

//}}}

// IU//{{{
IU::ptr IU::create(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	IU::ptr iu = IU::ptr(new IU(category, access_mode, read_only, payload_type)); /* params */ //));
	iu->_payload.initialize(iu);
	return iu;
}

IU::IU(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
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

void IU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
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
void IU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	if (is_published()) {
		_buffer->_send_iu_payload_update(this, is_delta, _revision, new_items, keys_to_remove, writer_name);
	}
	_revision_lock.unlock();
}

void IU::commit()
{
	_internal_commit();
}

void IU::_internal_commit(const std::string& writer_name)
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

RemotePushIU::ptr RemotePushIU::create()
{
	RemotePushIU::ptr iu = RemotePushIU::ptr(new RemotePushIU(/* params */));
	iu->_payload.initialize(iu);
	return iu;
}
RemotePushIU::RemotePushIU()
{
	// nothing
}
void RemotePushIU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
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
void RemotePushIU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
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

void RemotePushIU::commit()
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

void RemotePushIU::_apply_link_update(IULinkUpdate::ptr update)
{
	_revision = update->revision;
	if (update->is_delta) {
		_add_and_remove_links(update->new_links, update->links_to_remove);
	} else {
		_replace_links(update->new_links);
	}
}
void RemotePushIU::_apply_update(IUPayloadUpdate::ptr update)
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
void RemotePushIU::_apply_commission()
{
	_committed = true;
}
void RemotePushIU::_apply_retraction()
{
	_retracted = true;
}
//}}}

// RemoteMessage//{{{

RemoteMessage::ptr RemoteMessage::create()
{
	RemoteMessage::ptr iu = RemoteMessage::ptr(new RemoteMessage(/* params */));
	iu->_payload.initialize(iu);
	return iu;
}
RemoteMessage::RemoteMessage()
{
	// nothing
}
void RemoteMessage::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	IPAACA_INFO("Info: modifying a RemoteMessage only has local effects")
}
void RemoteMessage::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	IPAACA_INFO("Info: modifying a RemoteMessage only has local effects")
}
void RemoteMessage::commit()
{
	IPAACA_INFO("Info: committing to a RemoteMessage only has local effects")
}

void RemoteMessage::_apply_link_update(IULinkUpdate::ptr update)
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_link_update")
	_revision = update->revision;
	if (update->is_delta) {
		_add_and_remove_links(update->new_links, update->links_to_remove);
	} else {
		_replace_links(update->new_links);
	}
}
void RemoteMessage::_apply_update(IUPayloadUpdate::ptr update)
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
void RemoteMessage::_apply_commission()
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_commission")
	_committed = true;
}
void RemoteMessage::_apply_retraction()
{
	IPAACA_WARNING("Warning: should never be called: RemoteMessage::_apply_retraction")
	_retracted = true;
}

//}}}



// PayloadEntryProxy//{{{

PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key)
: _payload(payload), _key(key)
{
}
PayloadEntryProxy& PayloadEntryProxy::operator=(const std::string& value)
{
	//std::cout << "operator=(string)" << std::endl;
	_payload->set(_key, value);
	return *this;
}
PayloadEntryProxy& PayloadEntryProxy::operator=(const char* value)
{
	//std::cout << "operator=(const char*)" << std::endl;
	_payload->set(_key, value);
	return *this;
}
PayloadEntryProxy& PayloadEntryProxy::operator=(double value)
{
	//std::cout << "operator=(double)" << std::endl;
	_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}
PayloadEntryProxy& PayloadEntryProxy::operator=(bool value)
{
	//std::cout << "operator=(bool)" << std::endl;
	_payload->set(_key, boost::lexical_cast<std::string>(value));
	return *this;
}
PayloadEntryProxy::operator std::string()
{
	return _payload->get(_key);
}
PayloadEntryProxy::operator bool()
{
	std::string s = operator std::string();
	return ((s=="1")||(s=="true")||(s=="True"));
}
PayloadEntryProxy::operator long()
{
	//return boost::lexical_cast<long>(operator std::string().c_str());
	return atof(operator std::string().c_str());
}
PayloadEntryProxy::operator double()
{
	//return boost::lexical_cast<double>(operator std::string().c_str());
	return atol(operator std::string().c_str());
}
//}}}

// Payload//{{{

void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = iu;
}

PayloadEntryProxy Payload::operator[](const std::string& key)
{
	//boost::shared_ptr<PayloadEntryProxy> p(new PayloadEntryProxy(this, key));
	return PayloadEntryProxy(this, key);
}
Payload::operator std::map<std::string, std::string>()
{
	return _store;
}

inline void Payload::_internal_set(const std::string& k, const std::string& v, const std::string& writer_name) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_new[k]=v;
	_iu->_modify_payload(true, _new, _remove, writer_name );
	_store[k] = v;
}
inline void Payload::_internal_remove(const std::string& k, const std::string& writer_name) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu->_modify_payload(true, _new, _remove, writer_name );
	_store.erase(k);
}
void Payload::_internal_replace_all(const std::map<std::string, std::string>& new_contents, const std::string& writer_name)
{
	std::vector<std::string> _remove;
	_iu->_modify_payload(false, new_contents, _remove, writer_name );
	_store = new_contents;
}
inline std::string Payload::get(const std::string& k) {
	if (_store.count(k)>0) return _store[k];
	else return IPAACA_PAYLOAD_DEFAULT_STRING_VALUE;
}
void Payload::_remotely_enforced_wipe()
{
	_store.clear();
}
void Payload::_remotely_enforced_delitem(const std::string& k)
{
	_store.erase(k);
}
void Payload::_remotely_enforced_setitem(const std::string& k, const std::string& v)
{
	_store[k] = v;
}

//}}}

// IUConverter//{{{

IUConverter::IUConverter()
: Converter<std::string> ("ipaaca::IU", "ipaaca-iu", true)
{
}

std::string IUConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	//std::cout << "serialize" << std::endl;
	// Ensure that DATA actually holds a datum of the data-type we expect.
	assert(data.first == getDataType()); // "ipaaca::IU"
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const IU> obj = boost::static_pointer_cast<const IU> (data.second);
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid());
	pbo->set_revision(obj->revision());
	pbo->set_category(obj->category());
	pbo->set_payload_type(obj->payload_type());
	pbo->set_owner_name(obj->owner_name());
	pbo->set_committed(obj->committed());
	ipaaca::protobuf::IU_AccessMode a_m;
	switch(obj->access_mode()) {
		case IU_ACCESS_PUSH:
			a_m = ipaaca::protobuf::IU_AccessMode_PUSH;
			break;
		case IU_ACCESS_REMOTE:
			a_m = ipaaca::protobuf::IU_AccessMode_REMOTE;
			break;
		case IU_ACCESS_MESSAGE:
			a_m = ipaaca::protobuf::IU_AccessMode_MESSAGE;
			break;
	}
	pbo->set_access_mode(a_m);
	pbo->set_read_only(obj->read_only());
	for (std::map<std::string, std::string>::const_iterator it=obj->_payload._store.begin(); it!=obj->_payload._store.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_payload();
		item->set_key(it->first);
		item->set_value(it->second);
		item->set_type("str"); // FIXME other types than str (later)
	}
	for (LinkMap::const_iterator it=obj->_links._links.begin(); it!=obj->_links._links.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_links();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	pbo->SerializeToString(&wire);
	switch(obj->access_mode()) {
		case IU_ACCESS_PUSH:
			//std::cout << "Requesting to send as ipaaca-iu" << std::endl;
			return "ipaaca-iu";
		case IU_ACCESS_MESSAGE:
			//std::cout << "Requesting to send as ipaaca-messageiu" << std::endl;
			return "ipaaca-messageiu";
		default:
			//std::cout << "Requesting to send as default" << std::endl;
			return getWireSchema();
	}

}

AnnotatedData IUConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	//std::cout << "deserialize" << std::endl;
	assert(wireSchema == getWireSchema()); // "ipaaca-iu"
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	pbo->ParseFromString(wire);
	IUAccessMode mode = static_cast<IUAccessMode>(pbo->access_mode());
	switch(mode) {
		case IU_ACCESS_PUSH:
			{
			// Create a "remote push IU"
			boost::shared_ptr<RemotePushIU> obj = RemotePushIU::create();
			// transfer pbo data to obj
			obj->_uid = pbo->uid();
			obj->_revision = pbo->revision();
			obj->_category = pbo->category();
			obj->_payload_type = pbo->payload_type();
			obj->_owner_name = pbo->owner_name();
			obj->_committed = pbo->committed();
			obj->_read_only = pbo->read_only();
			obj->_access_mode = IU_ACCESS_PUSH;
			for (int i=0; i<pbo->payload_size(); i++) {
				const protobuf::PayloadItem& it = pbo->payload(i);
				obj->_payload._store[it.key()] = it.value();
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			//return std::make_pair(getDataType(), obj);
			return std::make_pair("ipaaca::RemotePushIU", obj);
			break;
			}
		case IU_ACCESS_MESSAGE:
			{
			// Create a "Message-type IU"
			boost::shared_ptr<RemoteMessage> obj = RemoteMessage::create();
			// transfer pbo data to obj
			obj->_uid = pbo->uid();
			obj->_revision = pbo->revision();
			obj->_category = pbo->category();
			obj->_payload_type = pbo->payload_type();
			obj->_owner_name = pbo->owner_name();
			obj->_committed = pbo->committed();
			obj->_read_only = pbo->read_only();
			obj->_access_mode = IU_ACCESS_MESSAGE;
			for (int i=0; i<pbo->payload_size(); i++) {
				const protobuf::PayloadItem& it = pbo->payload(i);
				obj->_payload._store[it.key()] = it.value();
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			//return std::make_pair(getDataType(), obj);
			return std::make_pair("ipaaca::RemoteMessage", obj);
			break;
			} 
		default:
			// other cases not handled yet! ( TODO )
			throw NotImplementedError();
	}
}

//}}}

// MessageConverter//{{{

MessageConverter::MessageConverter()
: Converter<std::string> ("ipaaca::Message", "ipaaca-messageiu", true)
{
}

std::string MessageConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	// Ensure that DATA actually holds a datum of the data-type we expect.
	assert(data.first == getDataType()); // "ipaaca::Message"
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const Message> obj = boost::static_pointer_cast<const Message> (data.second);
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid());
	pbo->set_revision(obj->revision());
	pbo->set_category(obj->category());
	pbo->set_payload_type(obj->payload_type());
	pbo->set_owner_name(obj->owner_name());
	pbo->set_committed(obj->committed());
	ipaaca::protobuf::IU_AccessMode a_m;
	switch(obj->access_mode()) {
		case IU_ACCESS_PUSH:
			a_m = ipaaca::protobuf::IU_AccessMode_PUSH;
			break;
		case IU_ACCESS_REMOTE:
			a_m = ipaaca::protobuf::IU_AccessMode_REMOTE;
			break;
		case IU_ACCESS_MESSAGE:
			a_m = ipaaca::protobuf::IU_AccessMode_MESSAGE;
			break;
	}
	pbo->set_access_mode(a_m);
	pbo->set_read_only(obj->read_only());
	for (std::map<std::string, std::string>::const_iterator it=obj->_payload._store.begin(); it!=obj->_payload._store.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_payload();
		item->set_key(it->first);
		item->set_value(it->second);
		item->set_type("str"); // FIXME other types than str (later)
	}
	for (LinkMap::const_iterator it=obj->_links._links.begin(); it!=obj->_links._links.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_links();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	pbo->SerializeToString(&wire);
	switch(obj->access_mode()) {
		case IU_ACCESS_PUSH:
			return "ipaaca-iu";
		case IU_ACCESS_MESSAGE:
			return "ipaaca-messageiu";
		default:
			//std::cout << "Requesting to send as default" << std::endl;
			return getWireSchema();
	}

}

AnnotatedData MessageConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu"
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	pbo->ParseFromString(wire);
	IUAccessMode mode = static_cast<IUAccessMode>(pbo->access_mode());
	switch(mode) {
		case IU_ACCESS_PUSH:
			{
			// Create a "remote push IU"
			boost::shared_ptr<RemotePushIU> obj = RemotePushIU::create();
			// transfer pbo data to obj
			obj->_uid = pbo->uid();
			obj->_revision = pbo->revision();
			obj->_category = pbo->category();
			obj->_payload_type = pbo->payload_type();
			obj->_owner_name = pbo->owner_name();
			obj->_committed = pbo->committed();
			obj->_read_only = pbo->read_only();
			obj->_access_mode = IU_ACCESS_PUSH;
			for (int i=0; i<pbo->payload_size(); i++) {
				const protobuf::PayloadItem& it = pbo->payload(i);
				obj->_payload._store[it.key()] = it.value();
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			//return std::make_pair(getDataType(), obj);
			return std::make_pair("ipaaca::RemotePushIU", obj);
			break;
			}
		case IU_ACCESS_MESSAGE:
			{
			// Create a "Message-type IU"
			boost::shared_ptr<RemoteMessage> obj = RemoteMessage::create();
			// transfer pbo data to obj
			obj->_uid = pbo->uid();
			obj->_revision = pbo->revision();
			obj->_category = pbo->category();
			obj->_payload_type = pbo->payload_type();
			obj->_owner_name = pbo->owner_name();
			obj->_committed = pbo->committed();
			obj->_read_only = pbo->read_only();
			obj->_access_mode = IU_ACCESS_MESSAGE;
			for (int i=0; i<pbo->payload_size(); i++) {
				const protobuf::PayloadItem& it = pbo->payload(i);
				obj->_payload._store[it.key()] = it.value();
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			//return std::make_pair(getDataType(), obj);
			return std::make_pair("ipaaca::RemoteMessage", obj);
			break;
			}
		default:
			// other cases not handled yet! ( TODO )
			throw NotImplementedError();
	}
}

//}}}

// IUPayloadUpdateConverter//{{{

IUPayloadUpdateConverter::IUPayloadUpdateConverter()
: Converter<std::string> ("ipaaca::IUPayloadUpdate", "ipaaca-iu-payload-update", true)
{
}

std::string IUPayloadUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IUPayloadUpdate"
	boost::shared_ptr<const IUPayloadUpdate> obj = boost::static_pointer_cast<const IUPayloadUpdate> (data.second);
	boost::shared_ptr<protobuf::IUPayloadUpdate> pbo(new protobuf::IUPayloadUpdate());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (std::map<std::string, std::string>::const_iterator it=obj->new_items.begin(); it!=obj->new_items.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_new_items();
		item->set_key(it->first);
		item->set_value(it->second);
		item->set_type("str"); // FIXME other types than str (later)
	}
	for (std::vector<std::string>::const_iterator it=obj->keys_to_remove.begin(); it!=obj->keys_to_remove.end(); ++it) {
		pbo->add_keys_to_remove(*it);
	}
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IUPayloadUpdateConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu-payload-update"
	boost::shared_ptr<protobuf::IUPayloadUpdate> pbo(new protobuf::IUPayloadUpdate());
	pbo->ParseFromString(wire);
	boost::shared_ptr<IUPayloadUpdate> obj(new IUPayloadUpdate());
	// transfer pbo data to obj
	obj->uid = pbo->uid();
	obj->revision = pbo->revision();
	obj->writer_name = pbo->writer_name();
	obj->is_delta = pbo->is_delta();
	for (int i=0; i<pbo->new_items_size(); i++) {
		const protobuf::PayloadItem& it = pbo->new_items(i);
		obj->new_items[it.key()] = it.value();
	}
	for (int i=0; i<pbo->keys_to_remove_size(); i++) {
		obj->keys_to_remove.push_back(pbo->keys_to_remove(i));
	}
	return std::make_pair(getDataType(), obj);
}

//}}}

// IULinkUpdateConverter//{{{

IULinkUpdateConverter::IULinkUpdateConverter()
: Converter<std::string> ("ipaaca::IULinkUpdate", "ipaaca-iu-link-update", true)
{
}

std::string IULinkUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IULinkUpdate"
	boost::shared_ptr<const IULinkUpdate> obj = boost::static_pointer_cast<const IULinkUpdate> (data.second);
	boost::shared_ptr<protobuf::IULinkUpdate> pbo(new protobuf::IULinkUpdate());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj->new_links.begin(); it!=obj->new_links.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_new_links();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj->links_to_remove.begin(); it!=obj->links_to_remove.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_links_to_remove();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IULinkUpdateConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu-link-update"
	boost::shared_ptr<protobuf::IULinkUpdate> pbo(new protobuf::IULinkUpdate());
	pbo->ParseFromString(wire);
	boost::shared_ptr<IULinkUpdate> obj(new IULinkUpdate());
	// transfer pbo data to obj
	obj->uid = pbo->uid();
	obj->revision = pbo->revision();
	obj->writer_name = pbo->writer_name();
	obj->is_delta = pbo->is_delta();
	for (int i=0; i<pbo->new_links_size(); ++i) {
		const protobuf::LinkSet& it = pbo->new_links(i);
		for (int j=0; j<it.targets_size(); ++j) {
			obj->new_links[it.type()].insert(it.targets(j)); // = vec;
		}
	}
	for (int i=0; i<pbo->links_to_remove_size(); ++i) {
		const protobuf::LinkSet& it = pbo->links_to_remove(i);
		for (int j=0; j<it.targets_size(); ++j) {
			obj->links_to_remove[it.type()].insert(it.targets(j));
		}
	}
	return std::make_pair(getDataType(), obj);
}

//}}}

// IntConverter//{{{

IntConverter::IntConverter()
: Converter<std::string> ("int", "int32", true)
{
}

std::string IntConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	// Ensure that DATA actually holds a datum of the data-type we expect.
	assert(data.first == getDataType()); // "int"
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const int> obj = boost::static_pointer_cast<const int> (data.second);
	boost::shared_ptr<protobuf::IntMessage> pbo(new protobuf::IntMessage());
	// transfer obj data to pbo
	pbo->set_value(*obj);
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IntConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "int"
	boost::shared_ptr<protobuf::IntMessage> pbo(new protobuf::IntMessage());
	pbo->ParseFromString(wire);
	boost::shared_ptr<int> obj = boost::shared_ptr<int>(new int(pbo->value()));
	return std::make_pair("int", obj);
}

//}}}

} // of namespace ipaaca


