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

#ifndef __ipaaca_buffers_h_INCLUDED__
#define __ipaaca_buffers_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif


/// store for (local) IUs. TODO Stores need to be unified more
IPAACA_HEADER_EXPORT class IUStore: public std::map<std::string, boost::shared_ptr<IU> >
{
};
/// store for RemotePushIUs. TODO Stores need to be unified more
IPAACA_HEADER_EXPORT class RemotePushIUStore: public std::map<std::string, boost::shared_ptr<RemotePushIU> > // TODO genericize to all remote IU types
{
};

typedef std::set<std::string> LinkSet;
typedef std::map<std::string, LinkSet> LinkMap;

IPAACA_HEADER_EXPORT class SmartLinkMap {
	friend std::ostream& operator<<(std::ostream& os, const SmartLinkMap& obj);
	friend class IUInterface;
	friend class IU;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_HEADER_EXPORT const LinkSet& get_links(const std::string& key);
		IPAACA_HEADER_EXPORT const LinkMap& get_all_links();

	protected:
		IPAACA_MEMBER_VAR_EXPORT LinkMap _links;
		IPAACA_MEMBER_VAR_EXPORT static LinkSet empty_link_set;
		IPAACA_HEADER_EXPORT void _add_and_remove_links(const LinkMap& add, const LinkMap& remove);
		IPAACA_HEADER_EXPORT void _replace_links(const LinkMap& links);
};

IPAACA_MEMBER_VAR_EXPORT const LinkSet EMPTY_LINK_SET;

// BufferConfiguration
IPAACA_HEADER_EXPORT class BufferConfiguration
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _basename;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> _category_interests;
		IPAACA_MEMBER_VAR_EXPORT std::string _channel;
	public:
		IPAACA_HEADER_EXPORT inline BufferConfiguration(const std::string basename) { _basename = basename; }
		IPAACA_HEADER_EXPORT const std::string get_basename() const { return _basename; }
		IPAACA_HEADER_EXPORT const std::vector<std::string> get_category_interests() const { return _category_interests; }
		IPAACA_HEADER_EXPORT const std::string get_channel() const { return _channel; }
};

// BufferConfigurationBuilder
IPAACA_HEADER_EXPORT class BufferConfigurationBuilder: private BufferConfiguration
{
	public:
		IPAACA_HEADER_EXPORT inline BufferConfigurationBuilder(const std::string basename):BufferConfiguration(basename) {}
		IPAACA_HEADER_EXPORT inline void set_basename(const std::string& basename)
		{
			_basename = basename;
		}
		IPAACA_HEADER_EXPORT inline void add_category_interest(const std::string& category)
		{
			_category_interests.push_back(category);
		}
		IPAACA_HEADER_EXPORT inline void set_channel(const std::string& channel)
		{
			_channel = channel;
		}

		IPAACA_HEADER_EXPORT const BufferConfiguration& get_buffer_configuration() { return *this; }

};

IPAACA_HEADER_EXPORT typedef boost::function<void (boost::shared_ptr<IUInterface>, IUEventType, bool)> IUEventHandlerFunction;

IPAACA_HEADER_EXPORT class IUEventHandler {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT IUEventHandlerFunction _function;
		IPAACA_MEMBER_VAR_EXPORT IUEventType _event_mask;
		IPAACA_MEMBER_VAR_EXPORT bool _for_all_categories;
		IPAACA_MEMBER_VAR_EXPORT std::set<std::string> _categories;
	protected:
		IPAACA_HEADER_EXPORT inline bool _condition_met(IUEventType event_type, const std::string& category)
		{
			return ((_event_mask&event_type)!=0) && (_for_all_categories || (_categories.count(category)>0));
		}
	public:
		IPAACA_HEADER_EXPORT IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::string& category);
		IPAACA_HEADER_EXPORT IUEventHandler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		//void call(Buffer* buffer, const std::string& uid, bool local, IUEventType event_type, const std::string& category);
		IPAACA_HEADER_EXPORT void call(Buffer* buffer, boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	typedef boost::shared_ptr<IUEventHandler> ptr;
};//}}}

	IPAACA_HEADER_EXPORT class Buffer { //: public boost::enable_shared_from_this<Buffer> {//{{{
		friend class IU;
		friend class RemotePushIU;
		friend class CallbackIUPayloadUpdate;
		friend class CallbackIULinkUpdate;
		friend class CallbackIUCommission;
		friend class CallbackIUResendRequest;
		protected:
			//Lock _handler_lock;
			IPAACA_MEMBER_VAR_EXPORT std::string _uuid;
			IPAACA_MEMBER_VAR_EXPORT std::string _basename;
			IPAACA_MEMBER_VAR_EXPORT std::string _unique_name;
			IPAACA_MEMBER_VAR_EXPORT std::string _id_prefix;
			IPAACA_MEMBER_VAR_EXPORT std::string _channel;
			IPAACA_MEMBER_VAR_EXPORT std::vector<IUEventHandler::ptr> _event_handlers;
		protected:
			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name) = 0;


			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef") = 0;
			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") = 0;
			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
	//		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
			IPAACA_HEADER_EXPORT void _allocate_unique_name(const std::string& basename, const std::string& function);
			IPAACA_HEADER_EXPORT inline Buffer(const std::string& basename, const std::string& function) {
				_allocate_unique_name(basename, function);
				_channel = "default";
			}
			IPAACA_HEADER_EXPORT void call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
		public:
			IPAACA_HEADER_EXPORT virtual inline ~Buffer() { }
			IPAACA_HEADER_EXPORT inline const std::string& unique_name() { return _unique_name; }
			IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
			IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask = IU_ALL_EVENTS, const std::string& category="");
			//_IPAACA_ABSTRACT_ virtual void add(boost::shared_ptr<IUInterface> iu) = 0;
			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual boost::shared_ptr<IUInterface> get(const std::string& iu_uid) = 0;
			IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual std::set<boost::shared_ptr<IUInterface> > get_ius() = 0;

			IPAACA_HEADER_EXPORT inline const std::string& channel() { return _channel; }
	};
	//}}}

	IPAACA_HEADER_EXPORT class OutputBuffer: public Buffer { //, public boost::enable_shared_from_this<OutputBuffer>  {//{{{
		friend class IU;
		friend class RemotePushIU;
		friend class OutputBufferRsbAdaptor;
		protected:
		protected:
			//OutputBufferRsbAdaptor _rsb;
			IPAACA_MEMBER_VAR_EXPORT IUStore _iu_store;
			IPAACA_MEMBER_VAR_EXPORT Lock _iu_id_counter_lock;
#ifdef IPAACA_EXPOSE_FULL_RSB_API
		protected:
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::Informer<rsb::AnyType>::Ptr> _informer_store;
			IPAACA_MEMBER_VAR_EXPORT rsb::patterns::ServerPtr _server;
			IPAACA_HEADER_EXPORT rsb::Informer<rsb::AnyType>::Ptr _get_informer(const std::string& category);
#endif
		protected:
			// informing functions
			IPAACA_HEADER_EXPORT void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef");
	IPAACA_HEADER_EXPORT void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

			IPAACA_HEADER_EXPORT void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef");
			IPAACA_HEADER_EXPORT void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name);
			IPAACA_HEADER_EXPORT void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name);
			// remote access functions
			// _remote_update_links(IULinkUpdate)
			// _remote_update_payload(IUPayloadUpdate)
			// _remote_commit(protobuf::IUCommission)
			IPAACA_HEADER_EXPORT void _publish_iu(boost::shared_ptr<IU> iu);

			IPAACA_HEADER_EXPORT void _retract_iu(boost::shared_ptr<IU> iu);
		protected:
			IPAACA_HEADER_EXPORT OutputBuffer(const std::string& basename, const std::string& channel="default");
			IPAACA_HEADER_EXPORT void _initialize_server();
		public:
			IPAACA_HEADER_EXPORT static boost::shared_ptr<OutputBuffer> create(const std::string& basename);
			IPAACA_HEADER_EXPORT ~OutputBuffer() {
				IPAACA_IMPLEMENT_ME
			}
			IPAACA_HEADER_EXPORT void add(boost::shared_ptr<IU> iu);
			IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(const std::string& iu_uid);
			IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(boost::shared_ptr<IU> iu);
			IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
			IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius();
		typedef boost::shared_ptr<OutputBuffer> ptr;
	};
	//}}}

	IPAACA_HEADER_EXPORT class InputBuffer: public Buffer { //, public boost::enable_shared_from_this<InputBuffer>  {//{{{
		friend class IU;
		friend class RemotePushIU;
		friend class InputBufferRsbAdaptor;
			//InputBufferRsbAdaptor _rsb;
#ifdef IPAACA_EXPOSE_FULL_RSB_API
		protected:
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::ListenerPtr> _listener_store;
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, rsb::patterns::RemoteServerPtr> _remote_server_store;
			IPAACA_MEMBER_VAR_EXPORT RemotePushIUStore _iu_store;  // TODO genericize
			IPAACA_HEADER_EXPORT rsb::patterns::RemoteServerPtr _get_remote_server(const std::string& unique_server_name);
			IPAACA_HEADER_EXPORT rsb::ListenerPtr _create_category_listener_if_needed(const std::string& category);
			IPAACA_HEADER_EXPORT void _handle_iu_events(rsb::EventPtr event);
			IPAACA_HEADER_EXPORT void _trigger_resend_request(rsb::EventPtr event);
#endif
		protected:
			IPAACA_HEADER_EXPORT inline void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef")
			{
				IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_link_update() should never be invoked")
			}
			IPAACA_HEADER_EXPORT inline void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name)
			{
				IPAACA_WARNING("(ERROR) InputBuffer::_publish_iu_resend() should never be invoked")
			}
			IPAACA_HEADER_EXPORT inline void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef")
			{
				IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_payload_update() should never be invoked")
			}
			IPAACA_HEADER_EXPORT inline void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
			{
				IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_commission() should never be invoked")
			}
			IPAACA_HEADER_EXPORT inline void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
			{
				IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_resendrequest() should never be invoked")
			}
		protected:
			IPAACA_HEADER_EXPORT InputBuffer(const BufferConfiguration& bufferconfiguration);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::set<std::string>& category_interests);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
			IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);

			IPAACA_MEMBER_VAR_EXPORT bool triggerResend;
			IPAACA_HEADER_EXPORT void set_resend(bool resendActive);
			IPAACA_HEADER_EXPORT bool get_resend();

		public:
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const BufferConfiguration& bufferconfiguration);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::set<std::string>& category_interests);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::vector<std::string>& category_interests);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
			IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);
			IPAACA_HEADER_EXPORT ~InputBuffer() {
				IPAACA_IMPLEMENT_ME
			}
			IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid);
			IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius();
		typedef boost::shared_ptr<InputBuffer> ptr;
	};
	//}}}

	IPAACA_HEADER_EXPORT class IUPayloadUpdate {//{{{
		public:
			IPAACA_MEMBER_VAR_EXPORT std::string uid;
			IPAACA_MEMBER_VAR_EXPORT revision_t revision;
			IPAACA_MEMBER_VAR_EXPORT std::string writer_name;
			IPAACA_MEMBER_VAR_EXPORT bool is_delta;
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> new_items;
			IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> keys_to_remove;
		friend std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj);
		typedef boost::shared_ptr<IUPayloadUpdate> ptr;
	};//}}}

	IPAACA_HEADER_EXPORT class IULinkUpdate {//{{{
		public:
			IPAACA_MEMBER_VAR_EXPORT std::string uid;
			IPAACA_MEMBER_VAR_EXPORT revision_t revision;
			IPAACA_MEMBER_VAR_EXPORT std::string writer_name;
			IPAACA_MEMBER_VAR_EXPORT bool is_delta;
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::set<std::string> > new_links;
			IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::set<std::string> > links_to_remove;
		friend std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj);
		typedef boost::shared_ptr<IULinkUpdate> ptr;
	};//}}}


#endif
