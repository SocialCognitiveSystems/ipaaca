#include <ipaaca.h>
#include <cstdlib>

namespace ipaaca {
/*
*/

void initialize_ipaaca_rsb()
{
	ParticipantConfig config = ParticipantConfig::fromConfiguration();
	Factory::getInstance().setDefaultParticipantConfig(config);
	
	boost::shared_ptr<IUPayloadUpdateConverter> payload_update_converter(new IUPayloadUpdateConverter());
	boost::shared_ptr<IULinkUpdateConverter> link_update_converter(new IULinkUpdateConverter());
	stringConverterRepository()->registerConverter(payload_update_converter);
	stringConverterRepository()->registerConverter(link_update_converter);
	
	//IPAACA_TODO("initialize all converters")
}

std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj)
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

std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj)
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

IU::ref IU::create(/* params */)
{
	IU::ref iu = IU::ref(new IU(/* params */));
	iu->payload.initialize(iu);
	return iu;
}

// PayloadEntryProxy//{{{

PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key)
: _payload(payload), _key(key)
{
}
PayloadEntryProxy& PayloadEntryProxy::operator=(const std::string& value)
{
	_payload->set(_key, value);
	return *this;
}
PayloadEntryProxy::operator std::string()
{
	return _payload->get(_key);
}
PayloadEntryProxy::operator long()
{
	return atol(operator std::string().c_str());
}
PayloadEntryProxy::operator double()
{
	return atof(operator std::string().c_str());
}
//}}}

// Payload//{{{

Payload::Payload()
{
}
void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = iu;
}

PayloadEntryProxy Payload::operator[](const std::string& key)
{
	//boost::shared_ptr<PayloadEntryProxy> p(new PayloadEntryProxy(this, key));
	return PayloadEntryProxy(this, key);
}

inline void Payload::set(const std::string& k, const std::string& v) {
	//self._iu._modify_payload(self, isdelta=true, newitm={k:v}, keystorm=[], writer_name=None );
	_store[k] = v;
}
inline void Payload::remove(const std::string& k) {
	//self._iu._modify_payload(self, isdelta=true, newitm={}, keystorm=[k], writer_name=None );
	_store.erase(k);
}
inline std::string Payload::get(const std::string& k) {
	if (_store.count(k)>0) return _store[k];
	else return IPAACA_PAYLOAD_DEFAULT_STRING_VALUE;
}
//}}}

/*

// IUConverter//{{{

IUConverter::IUConverter()
: Converter<std::string> ("ipaaca::IU", "ipaaca-iu", true)
{
}

std::string IUConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	// Ensure that DATA actually holds a datum of the data-type we expect.
	assert(data.first == getDataType()); // "ipaaca::IU"
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const IU> obj = boost::static_pointer_cast<const IU> (data.second);
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (std::map<std::string, std::string>::const_iterator it=obj->new_items.begin(); it!=obj->new_items.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_new_items();
		item->set_key(it->first);
		item->set_value(it->second);
	}
	for (std::vector<std::string>::const_iterator it=obj->keys_to_remove.begin(); it!=obj->keys_to_remove.end(); ++it) {
		pbo->add_keys_to_remove(*it);
	}
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IUConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu-payload-update"
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	pbo->ParseFromString(wire);
	boost::shared_ptr<IU> obj(new IU());
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

*/


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













} // of namespace ipaaca


