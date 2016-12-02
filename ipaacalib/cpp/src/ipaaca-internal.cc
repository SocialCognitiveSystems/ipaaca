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

// static library Initializer
IPAACA_EXPORT bool Initializer::_initialized = false;
IPAACA_EXPORT bool Initializer::initialized() { return _initialized; }
IPAACA_EXPORT void Initializer::initialize_ipaaca_rsb_if_needed()
{
	initialize_backend();
}
IPAACA_EXPORT void Initializer::initialize_backend()//{{{
{
	if (_initialized) return;

	auto_configure_rsb();

	IPAACA_DEBUG("Creating and registering Converters")
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

	// dlw
	boost::shared_ptr<ProtocolBufferConverter<protobuf::IUResendRequest> > iu_resendrequest_converter(new ProtocolBufferConverter<protobuf::IUResendRequest> ());
	converterRepository<std::string>()->registerConverter(iu_resendrequest_converter);

	boost::shared_ptr<ProtocolBufferConverter<protobuf::IURetraction> > iu_retraction_converter(new ProtocolBufferConverter<protobuf::IURetraction> ());
	converterRepository<std::string>()->registerConverter(iu_retraction_converter);

	boost::shared_ptr<IntConverter> int_converter(new IntConverter());
	converterRepository<std::string>()->registerConverter(int_converter);

	IPAACA_DEBUG("Backend / converter initialization complete.")
	_initialized = true;
}//}}}
IPAACA_EXPORT void Initializer::dump_current_default_config()//{{{
{
	IPAACA_INFO("--- Dumping current default participant configuration ---")
	rsb::ParticipantConfig config = getFactory().getDefaultParticipantConfig();
	std::set<rsb::ParticipantConfig::Transport> transports = config.getTransports();
	for (std::set<rsb::ParticipantConfig::Transport>::const_iterator it=transports.begin(); it!=transports.end(); ++it) {
		IPAACA_INFO( "Active transport: " << it->getName() )
	}
	IPAACA_INFO("--- End of configuration dump ---")
	//ParticipantConfig::Transport inprocess = config.getTransport("inprocess");
	//inprocess.setEnabled(true);
	//config.addTransport(inprocess);
}//}}}
IPAACA_EXPORT void Initializer::auto_configure_rsb()//{{{
{
	// set RSB host and port iff provided using cmdline arguments
	if (__ipaaca_static_option_rsb_host!="") {
		IPAACA_INFO("Overriding RSB host with " << __ipaaca_static_option_rsb_host)
		IPAACA_SETENV("RSB_TRANSPORT_SPREAD_HOST", __ipaaca_static_option_rsb_host.c_str())
		IPAACA_SETENV("RSB_TRANSPORT_SOCKET_HOST", __ipaaca_static_option_rsb_host.c_str());
	}
	if (__ipaaca_static_option_rsb_port!="") {
		IPAACA_INFO("Overriding RSB port with " << __ipaaca_static_option_rsb_port)
		IPAACA_SETENV("RSB_TRANSPORT_SPREAD_PORT", __ipaaca_static_option_rsb_port.c_str());
		IPAACA_SETENV("RSB_TRANSPORT_SOCKET_PORT", __ipaaca_static_option_rsb_port.c_str());
	}
	if (__ipaaca_static_option_rsb_transport!="") {
		if (__ipaaca_static_option_rsb_transport == "spread") {
			IPAACA_INFO("Overriding RSB transport mode - using 'spread' ")
			IPAACA_SETENV("RSB_TRANSPORT_SPREAD_ENABLED", "1", 1);
			IPAACA_SETENV("RSB_TRANSPORT_SOCKET_ENABLED", "0", 1);
		} else if (__ipaaca_static_option_rsb_transport == "socket") {
			IPAACA_INFO("Overriding RSB transport mode - using 'socket' ")
			IPAACA_SETENV("RSB_TRANSPORT_SPREAD_ENABLED", "0", 1);
			IPAACA_SETENV("RSB_TRANSPORT_SOCKET_ENABLED", "1", 1);
			if (__ipaaca_static_option_rsb_socketserver!="") {
				const std::string& srv = __ipaaca_static_option_rsb_socketserver;
				if ((srv=="1")||(srv=="0")||(srv=="auto")) {
					IPAACA_INFO("Overriding RSB transport.socket.server with " << srv)
					IPAACA_SETENV("RSB_TRANSPORT_SOCKET_SERVER", srv.c_str());
				} else {
					IPAACA_INFO("Unknown RSB transport.socket.server mode " << srv << " - using config default ")
				}
			}
		} else {
			IPAACA_INFO("Unknown RSB transport mode " <<  __ipaaca_static_option_rsb_transport << " - using config default ")
		}
	}
	
	const char* plugin_path = getenv("RSB_PLUGINS_CPP_PATH");
	if (!plugin_path) {
#ifdef WIN32
		IPAACA_WARNING("WARNING: RSB_PLUGINS_CPP_PATH not set - in Windows it has to be specified.")
		//throw NotImplementedError();
#else
		IPAACA_INFO("RSB_PLUGINS_CPP_PATH not set; looking here and up to 7 dirs up.")
		std::string pathstr = "./";
		for (int i=0; i<   8 /* depth EIGHT (totally arbitrary..) */  ; i++) {
			std::string where_str = pathstr+"deps/lib/rsb*/plugins";
			const char* where = where_str.c_str();
			glob_t g;
			glob(where, 0, NULL, &g);
			if (g.gl_pathc>0) {
				const char* found_path = g.gl_pathv[0];
				IPAACA_INFO("Found an RSB plugin dir which will be used automatically: " << found_path)
				IPAACA_SETENV("RSB_PLUGINS_CPP_PATH", found_path);
				break;
			} // else keep going
			globfree(&g);
			pathstr += "../";
		}
#endif
	} else {
		IPAACA_INFO("RSB_PLUGINS_CPP_PATH already defined: " << plugin_path)
	}
}//}}}

// RSB backend Converters
// IUConverter//{{{

IPAACA_EXPORT IUConverter::IUConverter()
: Converter<std::string> (IPAACA_SYSTEM_DEPENDENT_CLASS_NAME("ipaaca::IU"), "ipaaca-iu", true)
{
}

IPAACA_EXPORT std::string IUConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IU"
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
	for (auto& kv: obj->_payload._document_store) {
		protobuf::PayloadItem* item = pbo->add_payload();
		item->set_key(kv.first);
		IPAACA_DEBUG("Payload type: " << obj->_payload_type)
		if (obj->_payload_type=="JSON") {
			item->set_value( kv.second->to_json_string_representation() );
			item->set_type("JSON");
		} else if ((obj->_payload_type=="MAP") || (obj->_payload_type=="STR")) {
			// legacy mode
			item->set_value( json_value_cast<std::string>(kv.second->document));
			item->set_type("STR");
		}
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
			return getWireSchema();
	}

}

IPAACA_EXPORT AnnotatedData IUConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
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
				PayloadDocumentEntry::ptr entry;
				if (it.type() == "JSON") {
					// fully parse json text
					entry = PayloadDocumentEntry::from_json_string_representation( it.value() );
				} else {
					// assuming legacy "str" -> just copy value to raw string in document
					entry = std::make_shared<PayloadDocumentEntry>();
					entry->document.SetString(it.value(), entry->document.GetAllocator());
				}
				obj->_payload._document_store[it.key()] = entry;
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			return std::make_pair("ipaaca::RemotePushIU", obj);
			break;
			}
		case IU_ACCESS_MESSAGE:
			{
			// Create a "Message-type IU"
			boost::shared_ptr<RemoteMessage> obj = RemoteMessage::create();
			//std::cout << "REFCNT after create: " << obj.use_count() << std::endl;
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
				PayloadDocumentEntry::ptr entry;
				if (it.type() == "JSON") {
					// fully parse json text
					entry = PayloadDocumentEntry::from_json_string_representation( it.value() );
				} else {
					// assuming legacy "str" -> just copy value to raw string in document
					entry = std::make_shared<PayloadDocumentEntry>();
					entry->document.SetString(it.value(), entry->document.GetAllocator());
				}
				obj->_payload._document_store[it.key()] = entry;
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			return std::make_pair("ipaaca::RemoteMessage", obj);
			break;
			}
		default:
			// no other cases (yet)
			throw NotImplementedError();
	}
}

//}}}
// MessageConverter//{{{

IPAACA_EXPORT MessageConverter::MessageConverter()
: Converter<std::string> (IPAACA_SYSTEM_DEPENDENT_CLASS_NAME("ipaaca::Message"), "ipaaca-messageiu", true)
{
}

IPAACA_EXPORT std::string MessageConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::Message"
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
	for (auto& kv: obj->_payload._document_store) {
		protobuf::PayloadItem* item = pbo->add_payload();
		item->set_key(kv.first);
		if (obj->_payload_type=="JSON") {
			item->set_value( kv.second->to_json_string_representation() );
			item->set_type("JSON");
		} else if ((obj->_payload_type=="MAP") || (obj->_payload_type=="STR")) {
			// legacy mode
			item->set_value( json_value_cast<std::string>(kv.second->document));
			item->set_type("STR");
		}
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
			return getWireSchema();
	}

}

IPAACA_EXPORT AnnotatedData MessageConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
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
				PayloadDocumentEntry::ptr entry;
				if (it.type() == "JSON") {
					// fully parse json text
					entry = PayloadDocumentEntry::from_json_string_representation( it.value() );
				} else {
					// assuming legacy "str" -> just copy value to raw string in document
					entry = std::make_shared<PayloadDocumentEntry>();
					entry->document.SetString(it.value(), entry->document.GetAllocator());
				}
				obj->_payload._document_store[it.key()] = entry;
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
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
				PayloadDocumentEntry::ptr entry;
				if (it.type() == "JSON") {
					// fully parse json text
					entry = PayloadDocumentEntry::from_json_string_representation( it.value() );
				} else {
					// assuming legacy "str" -> just copy value to raw string in document
					entry = std::make_shared<PayloadDocumentEntry>();
					entry->document.SetString(it.value(), entry->document.GetAllocator());
				}
				obj->_payload._document_store[it.key()] = entry;
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			return std::make_pair("ipaaca::RemoteMessage", obj);
			break;
			}
		default:
			// no other cases (yet)
			throw NotImplementedError();
	}
}

//}}}
// IUPayloadUpdateConverter//{{{

IPAACA_EXPORT IUPayloadUpdateConverter::IUPayloadUpdateConverter()
: Converter<std::string> (IPAACA_SYSTEM_DEPENDENT_CLASS_NAME("ipaaca::IUPayloadUpdate"), "ipaaca-iu-payload-update", true)
{
}

IPAACA_EXPORT std::string IUPayloadUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IUPayloadUpdate"
	boost::shared_ptr<const IUPayloadUpdate> obj = boost::static_pointer_cast<const IUPayloadUpdate> (data.second);
	boost::shared_ptr<protobuf::IUPayloadUpdate> pbo(new protobuf::IUPayloadUpdate());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (auto& kv: obj->new_items) {
		protobuf::PayloadItem* item = pbo->add_new_items();
		item->set_key(kv.first);
		if (obj->payload_type=="JSON") {
			item->set_value( kv.second->to_json_string_representation() );
			item->set_type("JSON");
		} else if ((obj->payload_type=="MAP") || (obj->payload_type=="STR")) {
			// legacy mode
			item->set_value( json_value_cast<std::string>(kv.second->document));
			item->set_type("STR");
		} else {
			IPAACA_ERROR("Uninitialized payload update type!")
			throw NotImplementedError();
		}
		IPAACA_DEBUG("Adding updated item (type " << item->type() << "): " << item->key() << " -> " << item->value() )
	}
	for (auto& key: obj->keys_to_remove) {
		pbo->add_keys_to_remove(key);
		IPAACA_DEBUG("Adding removed key: " << key)
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
		PayloadDocumentEntry::ptr entry;
		if (it.type() == "JSON") {
			// fully parse json text
			entry = PayloadDocumentEntry::from_json_string_representation( it.value() );
			IPAACA_INFO("New/updated payload entry: " << it.key() << " -> " << it.value() )
		} else {
			// assuming legacy "str" -> just copy value to raw string in document
			entry = std::make_shared<PayloadDocumentEntry>();
			entry->document.SetString(it.value(), entry->document.GetAllocator());
		}
		obj->new_items[it.key()] = entry;
	}
	for (int i=0; i<pbo->keys_to_remove_size(); i++) {
		obj->keys_to_remove.push_back(pbo->keys_to_remove(i));
	}
	return std::make_pair(getDataType(), obj);
}

//}}}
// IULinkUpdateConverter//{{{

IPAACA_EXPORT IULinkUpdateConverter::IULinkUpdateConverter()
: Converter<std::string> (IPAACA_SYSTEM_DEPENDENT_CLASS_NAME("ipaaca::IULinkUpdate"), "ipaaca-iu-link-update", true)
{
}

IPAACA_EXPORT std::string IULinkUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType());
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

IPAACA_EXPORT IntConverter::IntConverter()
: Converter<std::string> ("int", "int32", true)
{
}

IPAACA_EXPORT std::string IntConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType());
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const int> obj = boost::static_pointer_cast<const int> (data.second);
	boost::shared_ptr<protobuf::IntMessage> pbo(new protobuf::IntMessage());
	// transfer obj data to pbo
	pbo->set_value(*obj);
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

IPAACA_EXPORT AnnotatedData IntConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "int"
	boost::shared_ptr<protobuf::IntMessage> pbo(new protobuf::IntMessage());
	pbo->ParseFromString(wire);
	boost::shared_ptr<int> obj = boost::shared_ptr<int>(new int(pbo->value()));
	return std::make_pair("int", obj);
}

//}}}

} // of namespace ipaaca
