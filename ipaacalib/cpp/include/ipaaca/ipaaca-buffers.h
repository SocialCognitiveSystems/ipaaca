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

/// Container for IU links that gracefully returns the empty set if required
IPAACA_HEADER_EXPORT class SmartLinkMap {//{{{
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
};//}}}

/// The empty link set is returned if undefined links are read for an IU.
IPAACA_MEMBER_VAR_EXPORT const LinkSet EMPTY_LINK_SET;

/// Configuration object that can be passed to Buffer constructors.
IPAACA_HEADER_EXPORT class BufferConfiguration//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _basename;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> _category_interests;
		IPAACA_MEMBER_VAR_EXPORT std::string _channel;
	public:
		IPAACA_HEADER_EXPORT inline BufferConfiguration(const std::string& basename): _basename(basename), _channel(__ipaaca_static_option_default_channel) { }
		IPAACA_HEADER_EXPORT inline const std::string& get_basename() const { return _basename; }
		IPAACA_HEADER_EXPORT inline const std::vector<std::string>& get_category_interests() const { return _category_interests; }
		IPAACA_HEADER_EXPORT inline const std::string& get_channel() const { return _channel; }
	public:
		// setters, initialization helpers
		IPAACA_HEADER_EXPORT inline BufferConfiguration& set_basename(const std::string& basename) { _basename = basename; return *this; }
		IPAACA_HEADER_EXPORT inline BufferConfiguration& add_category_interest(const std::string& category) { _category_interests.push_back(category); return *this; }
		IPAACA_HEADER_EXPORT inline BufferConfiguration& set_channel(const std::string& channel) { _channel = channel; return *this; }
};//}}}

/// Builder object for BufferConfiguration, not required for C++ [DEPRECATED]
IPAACA_HEADER_EXPORT class BufferConfigurationBuilder: private BufferConfiguration//{{{
{
	public:
		[[deprecated("Use setters in BufferConfiguration instead of the Builder")]]
		IPAACA_HEADER_EXPORT inline BufferConfigurationBuilder(const std::string& basename): BufferConfiguration(basename) {}
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

};//}}}

/** \brief Type of user-space functions that can be registered on a Buffer to receive IU events.
 *
 * The signature of these functions is void(shared_ptr<IUInterface> iu, IUEventType evt_type, bool local), where:<br/>
 *     iu can be used mostly like a locally-generated IU reference (e.g. iu->payload() ...)<br/>
 *     evt_type is one of IU_ADDED, IU_UPDATED, IU_RETRACTED, IU_DELETED, IU_LINKSUPDATED, IU_COMMITTED, IU_MESSAGE<br/>
 *     local indicates that a remote change to a local IU (in an OutputBuffer) was effected
 *
 *
 */
IPAACA_HEADER_EXPORT typedef boost::function<void (boost::shared_ptr<IUInterface>, IUEventType, bool)> IUEventHandlerFunction;

/** \brief Internal handler type used in Buffer (wraps used-specified IUEventHandlerFunction)
 */
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

/**
 * \brief Buffer base class. Derived classes use its handler registration functionality.
 *
 * \b Note: This class is never instantiated directly (use OutputBuffer and InputBuffer, respectively).
 */
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
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
//		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") = 0;
		IPAACA_HEADER_EXPORT void _allocate_unique_name(const std::string& basename, const std::string& function);
		IPAACA_HEADER_EXPORT inline Buffer(const std::string& basename, const std::string& function) {
			_allocate_unique_name(basename, function);
			_channel = __ipaaca_static_option_default_channel;
		}
		IPAACA_HEADER_EXPORT void call_iu_event_handlers(boost::shared_ptr<IUInterface> iu, bool local, IUEventType event_type, const std::string& category);
	public:
		IPAACA_HEADER_EXPORT virtual inline ~Buffer() { }
		IPAACA_HEADER_EXPORT inline const std::string& unique_name() { return _unique_name; }
		/// This version of register_handler takes a set of several category interests instead of just one.
		IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask, const std::set<std::string>& categories);
		/** \brief Register a user-specified handler for IU events.
		 *
		 * \param function A function [object] that can be converted to #IUEventHandlerFunction (examples below)
		 * \param event_mask Which event types to relay to the user (default: all)
		 * \param category The category to filter for (default: do not filter)
		 *
		 * \b Examples:
		 *
		 * Adding a plain function as a handler:<br/>
		 *     <pre>
		 *     void global_iu_handler(IUInterface::ptr iu, IUEventType type, bool local) { ... }
		 *     ...
		 *     int main() {
		 *         OutputBuffer::ptr outbuf = OutputBuffer::create("mybufname");
		 *         outbuf->register_handler(global_iu_handler);
		 *         ...
		 *     }
		 *     </pre>
		 *
		 * Adding a class member as a handler (using boost::bind):<br/>
		 *     <pre>
		 *     class MyClass {
		 *         protected:
		 *             void my_internal_iu_handler(IUInterface::ptr iu, IUEventType type, bool local) { ... }
		 *             InputBuffer::ptr inbuf;
		 *         public:
		 *             MyClass() {
		 *                 inbuf = InputBuffer::create("bufname", "categoryInterest");
		 *                 inbuf->register_handler(boost::bind(&MyClass::my_internal_iu_handler, this, _1, _2, _3));
		 *             }
		 *     };
		 *     </pre>
		 *
		 */
		IPAACA_HEADER_EXPORT void register_handler(IUEventHandlerFunction function, IUEventType event_mask = IU_ALL_EVENTS, const std::string& category="");
		//_IPAACA_ABSTRACT_ virtual void add(boost::shared_ptr<IUInterface> iu) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual boost::shared_ptr<IUInterface> get(const std::string& iu_uid) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual std::set<boost::shared_ptr<IUInterface> > get_ius() = 0;

		IPAACA_HEADER_EXPORT inline const std::string& channel() { return _channel; }
};
//}}}

/**
 * \brief A buffer to which own IUs can be added to publish them
 *
 * Use OutputBuffer::create() to obtain a smart pointer to a new output buffer.
 * 
 * Use OutputBuffer::add() to add (= publish) an IU.
 *
 * Use OutputBuffer::remove() to remove (= retract) an IU.
 *
 * Use Buffer::register_handler() to register a handler that will respond to remote changes to own published IUs.
 */
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
		IPAACA_HEADER_EXPORT void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name) _IPAACA_OVERRIDE_;

		IPAACA_HEADER_EXPORT void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string,  PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name) _IPAACA_OVERRIDE_;
		//IPAACA_HEADER_EXPORT void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name);
		// remote access functions
		// _remote_update_links(IULinkUpdate)
		// _remote_update_payload(IUPayloadUpdate)
		// _remote_commit(protobuf::IUCommission)
		IPAACA_HEADER_EXPORT void _publish_iu(boost::shared_ptr<IU> iu);

		IPAACA_HEADER_EXPORT void _retract_iu(boost::shared_ptr<IU> iu);
	protected:
		/// \b Note: constructor is protected. Use create()
		IPAACA_HEADER_EXPORT OutputBuffer(const std::string& basename, const std::string& channel=""); // empty string auto-replaced with __ipaaca_static_option_default_channel
		IPAACA_HEADER_EXPORT void _initialize_server();
	public:
		IPAACA_HEADER_EXPORT static boost::shared_ptr<OutputBuffer> create(const std::string& basename);
		IPAACA_HEADER_EXPORT ~OutputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT void add(boost::shared_ptr<IU> iu);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(const std::string& iu_uid);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IU> remove(boost::shared_ptr<IU> iu);
		IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid) _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius() _IPAACA_OVERRIDE_;
	typedef boost::shared_ptr<OutputBuffer> ptr;
};
//}}}

/**
 * \brief A buffer in which remote IUs (and changes to them) are received.
 *
 * Use InputBuffer::create() to obtain a smart pointer to a new input buffer.
 *
 * Set category interests (IU filter) via the different versions of create().
 *
 * Use Buffer::register_handler() to register a handler that will respond to relevant remote IUs.
 */
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
		IPAACA_HEADER_EXPORT inline void _send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name="undef") _IPAACA_OVERRIDE_
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_link_update() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _publish_iu_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name) _IPAACA_OVERRIDE_
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_publish_iu_resend() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string,  PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name="undef") _IPAACA_OVERRIDE_
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_payload_update() should never be invoked")
		}
		IPAACA_HEADER_EXPORT inline void _send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name="undef") _IPAACA_OVERRIDE_
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_commission() should never be invoked")
		}
		/*IPAACA_HEADER_EXPORT inline void _send_iu_resendrequest(IUInterface* iu, revision_t revision, const std::string& writer_name="undef")
		{
			IPAACA_WARNING("(ERROR) InputBuffer::_send_iu_resendrequest() should never be invoked")
		}*/
	protected:
		/// \b Note: all constructors are protected. Use create()
		IPAACA_HEADER_EXPORT InputBuffer(const BufferConfiguration& bufferconfiguration);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::set<std::string>& category_interests);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::vector<std::string>& category_interests);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		IPAACA_HEADER_EXPORT InputBuffer(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);

		IPAACA_MEMBER_VAR_EXPORT bool triggerResend;

	public:
		/// Specify whether old but previously unseen IUs should be requested to be sent to the buffer over a hidden channel.
		IPAACA_HEADER_EXPORT void set_resend(bool resendActive);
		IPAACA_HEADER_EXPORT bool get_resend();
		/// Create InputBuffer according to configuration in BufferConfiguration object
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const BufferConfiguration& bufferconfiguration);
		/// Create InputBuffer from name and set of category interests
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::set<std::string>& category_interests);
		/// Create InputBuffer from name and vector of category interests
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::vector<std::string>& category_interests);
		// /// Create InputBuffer from name and initializer_list of category interests
		// IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::initializer_list<std::string>& category_interests);
		/// Convenience function: create InputBuffer from name and one category interest
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1);
		/// Convenience function: create InputBuffer from name and two category interests [DEPRECATED]
		[[deprecated("Use create(string, set<string>) instead")]]
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2);
		/// Convenience function: create InputBuffer from name and three category interests [DEPRECATED]
		[[deprecated("Use create(string, set<string>) instead")]]
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3);
		/// Convenience function: create InputBuffer from name and four category interests [DEPRECATED]
		[[deprecated("Use create(string, set<string>) instead")]]
		IPAACA_HEADER_EXPORT static boost::shared_ptr<InputBuffer> create(const std::string& basename, const std::string& category_interest1, const std::string& category_interest2, const std::string& category_interest3, const std::string& category_interest4);
		IPAACA_HEADER_EXPORT ~InputBuffer() {
			IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT boost::shared_ptr<IUInterface> get(const std::string& iu_uid) _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT std::set<boost::shared_ptr<IUInterface> > get_ius() _IPAACA_OVERRIDE_;
	typedef boost::shared_ptr<InputBuffer> ptr;
};
//}}}

/// Internal, transport-independent, representation of payload updates
IPAACA_HEADER_EXPORT class IUPayloadUpdate {//{{{
	public:
		IPAACA_MEMBER_VAR_EXPORT std::string uid;
		IPAACA_MEMBER_VAR_EXPORT revision_t revision;
		IPAACA_MEMBER_VAR_EXPORT std::string writer_name;
		IPAACA_MEMBER_VAR_EXPORT bool is_delta;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, PayloadDocumentEntry::ptr> new_items;
		IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> keys_to_remove;
		IPAACA_MEMBER_VAR_EXPORT std::string payload_type; // to handle legacy mode
	friend std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj);
	typedef boost::shared_ptr<IUPayloadUpdate> ptr;
};//}}}

/// Internal, transport-independent, representation of link updates
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
