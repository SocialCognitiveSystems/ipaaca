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

/**
 * \file   ipaaca-ius.h
 *
 * \brief Header file for IU (incremental unit) classes.
 *
 * Users should not include this file directly, but use ipaaca.h
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#ifndef __ipaaca_ius_h_INCLUDED__
#define __ipaaca_ius_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif


/** \brief Abstract base class for all IU-type classes
 *
 * In user programs, classes IU or Message should be instantiated.
 *
 * This abstract type is used in handler callback functions and
 * contains most generic user-space functions.
 *
 */
IPAACA_HEADER_EXPORT class IUInterface {//{{{
	friend class IUConverter;
	friend class MessageConverter;
	friend std::ostream& operator<<(std::ostream& os, const IUInterface& obj);
	protected:
		IPAACA_HEADER_EXPORT IUInterface();
	public:
		IPAACA_HEADER_EXPORT inline virtual ~IUInterface() { }
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _uid;
		IPAACA_MEMBER_VAR_EXPORT revision_t _revision;
		IPAACA_MEMBER_VAR_EXPORT std::string _category;
		IPAACA_MEMBER_VAR_EXPORT std::string _payload_type; // default is taken from __ipaaca_static_option_default_payload_type
		IPAACA_MEMBER_VAR_EXPORT std::string _owner_name;
		IPAACA_MEMBER_VAR_EXPORT bool _committed;
		IPAACA_MEMBER_VAR_EXPORT bool _retracted;
		IPAACA_MEMBER_VAR_EXPORT IUAccessMode _access_mode;
		IPAACA_MEMBER_VAR_EXPORT bool _read_only;
		//boost::shared_ptr<Buffer> _buffer;
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
		IPAACA_MEMBER_VAR_EXPORT SmartLinkMap _links;
	protected:
		friend class Payload;
		// Internal functions that perform the update logic,
		//  e.g. sending a notification across the network
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name) = 0;
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name) = 0;
		//void _set_buffer(boost::shared_ptr<Buffer> buffer);
		IPAACA_HEADER_EXPORT void _associate_with_buffer(Buffer* buffer);
		IPAACA_HEADER_EXPORT void _set_buffer(Buffer* buffer);
		IPAACA_HEADER_EXPORT void _set_uid(const std::string& uid);
		IPAACA_HEADER_EXPORT void _set_owner_name(const std::string& owner_name);
	protected:
		// internal functions that do not emit update events
		IPAACA_HEADER_EXPORT void _add_and_remove_links(const LinkMap& add, const LinkMap& remove) { _links._add_and_remove_links(add, remove); }
		IPAACA_HEADER_EXPORT void _replace_links(const LinkMap& links) { _links._replace_links(links); }
	public:
		/// Return whether IU has already been published (is in a Buffer).
		IPAACA_HEADER_EXPORT inline bool is_published() { return (_buffer != 0); }
		/// Return auto-generated UID string (set during IU construction)
		IPAACA_HEADER_EXPORT inline const std::string& uid() const { return _uid; }
		/// Return current IU revision number (incremented for each update)
		IPAACA_HEADER_EXPORT inline revision_t revision() const { return _revision; }
		/// Return the IU category string (set during IU construction)
		IPAACA_HEADER_EXPORT inline const std::string& category() const { return _category; }
		/// Return the channel name the IU is resident on (set on publication)
		IPAACA_HEADER_EXPORT const std::string& channel();
		/// Return the payload type (default "JSON")
		IPAACA_HEADER_EXPORT inline const std::string& payload_type() const { return _payload_type; }
		/// Return the owner name (unique fully-qualified buffer name, set on publication)
		IPAACA_HEADER_EXPORT inline const std::string& owner_name() const { return _owner_name; }
		/// Return whether IU has been committed to (i.e. is complete, confirmed, and henceforth constant)
		IPAACA_HEADER_EXPORT inline bool committed() const { return _committed; }
		/// Return the access mode (not relevant for the time being)
		IPAACA_HEADER_EXPORT inline IUAccessMode access_mode() const { return _access_mode; }
		/// Return whether IU is read only (committed, a Message, or explicitly set read-only by owner)
		IPAACA_HEADER_EXPORT inline bool read_only() const { return _read_only; }
		//inline boost::shared_ptr<Buffer> buffer() { return _buffer; }
		/// Return owning buffer [CAVEAT: do not rely on this function for future code]
		IPAACA_HEADER_EXPORT inline Buffer* buffer() const { return _buffer; }
		/// Return the link set for an arbitrary link type (e.g. "grounded_in"), or EMPTY_LINK_SET
		IPAACA_HEADER_EXPORT inline const LinkSet& get_links(std::string type) { return _links.get_links(type); }
		/// Return the map of all defined links
		IPAACA_HEADER_EXPORT inline const LinkMap& get_all_links() { return _links.get_all_links(); }
		// Payload
		/// Return the Payload object of this IU, overridden in the derived classes
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual Payload& payload() = 0;
		/// Const version of payload()
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual const Payload& const_payload() const = 0;
		// setters
		/// Commit to an IU (only possible for the IU owner)
		IPAACA_HEADER_EXPORT _IPAACA_ABSTRACT_ virtual void commit() = 0;
		// functions to modify and update links:
		//IPAACA_HEADER_EXPORT void _publish_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

		/// Add a set of new UIDs for a specific link type
		IPAACA_HEADER_EXPORT void add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		/// Remove a set of UIDs from a link type
		IPAACA_HEADER_EXPORT void remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name = "");
		/// Bulk link modification function
		IPAACA_HEADER_EXPORT void modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name = "");
		/// Bulk link override function
		IPAACA_HEADER_EXPORT void set_links(const LinkMap& links, const std::string& writer_name = "");
		//    (with cpp specific convenience functions:)
		/// Convenience function (C++): add a single UID string to an arbitrary link set
		IPAACA_HEADER_EXPORT void add_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
		/// Convenience function (C++): remove a single UID string from an arbitrary link set (if contained)
		IPAACA_HEADER_EXPORT void remove_link(const std::string& type, const std::string& target, const std::string& writer_name = "");
	typedef boost::shared_ptr<IUInterface> ptr;
};//}}}

/** \brief Class of locally-owned IU objects.
 *
 * Use IU::create() (static) to create references to new IUs.
 * Use IU::payload() to access the IUs payload object.
 * Use OutputBuffer::add() to publish IUs.
 *
 * See IUInterface for a generic description of most user-space member functions.
 */
IPAACA_HEADER_EXPORT class IU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class CallbackIUPayloadUpdate;
	friend class CallbackIULinkUpdate;
	friend class CallbackIUCommission;
	friend class CallbackIUResendRequest;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
	   IPAACA_MEMBER_VAR_EXPORT  Lock _revision_lock;
	protected:
		IPAACA_HEADER_EXPORT inline void _increase_revision_number() { _revision++; }
		IPAACA_HEADER_EXPORT IU(const std::string& category, IUAccessMode access_mode=IU_ACCESS_PUSH, bool read_only=false, const std::string& payload_type="" ); // __ipaaca_static_option_default_payload_type
	public:
		IPAACA_HEADER_EXPORT inline ~IU() {
			//IPAACA_IMPLEMENT_ME
		}
		[[deprecated("Please use the new argument order: category, payload_type, read_only")]]
		IPAACA_HEADER_EXPORT static boost::shared_ptr<IU> create(const std::string& category, IUAccessMode access_mode, bool read_only=false, const std::string& payload_type="" );
		IPAACA_HEADER_EXPORT static boost::shared_ptr<IU> create(const std::string& category, const std::string& payload_type="", bool read_only=false);
		IPAACA_HEADER_EXPORT inline Payload& payload() _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT void commit() _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT virtual void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;

		//IPAACA_HEADER_EXPORT virtual void _publish_resend(boost::shared_ptr<IU> iu, const std::string& hidden_scope_name);

		IPAACA_HEADER_EXPORT virtual void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT virtual void _internal_commit(const std::string& writer_name = "");
	public:
	typedef boost::shared_ptr<IU> ptr;
};//}}}
/** \brief Class of locally-owned message objects ('one-shot' IUs).
 *
 * This class works the same as IU, except that it sets the internal
 * flags so that it is received as a message (ephemeral object) on
 * the remote sides, instead of a persistent objects.
 *
 * Likewise, it is not actually stored by OutputBuffer::add(), but just broadcast.
 * 
 * See IUInterface for a generic description of most user-space member functions.
 *
 * \see IU, IUInterface
 */
IPAACA_HEADER_EXPORT class Message: public IU {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class CallbackIUPayloadUpdate;
	friend class CallbackIULinkUpdate;
	friend class CallbackIUCommission;
	friend class CallbackIUResendRequest;
	protected:
		IPAACA_HEADER_EXPORT Message(const std::string& category, IUAccessMode access_mode=IU_ACCESS_MESSAGE, bool read_only=true, const std::string& payload_type="" );
	public:
		IPAACA_HEADER_EXPORT inline ~Message() {
			//IPAACA_IMPLEMENT_ME
		}
		[[deprecated("Please use the new argument order: category, payload_type")]]
		IPAACA_HEADER_EXPORT static boost::shared_ptr<Message> create(const std::string& category, IUAccessMode access_mode, bool read_only=true, const std::string& payload_type="" );
		IPAACA_HEADER_EXPORT static boost::shared_ptr<Message> create(const std::string& category, const std::string& payload_type="");
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _internal_commit(const std::string& writer_name = "");
	public:
	typedef boost::shared_ptr<Message> ptr;
};//}}}

/// Copy of a remote IU, received in an InputBuffer. Setter functions call RPC over the backend (RSB). \b Note: Typically handled only as reference in a handler in user space.
IPAACA_HEADER_EXPORT class RemotePushIU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
		IPAACA_HEADER_EXPORT RemotePushIU();
		IPAACA_HEADER_EXPORT static boost::shared_ptr<RemotePushIU> create();
	public:
		IPAACA_HEADER_EXPORT inline ~RemotePushIU() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT inline Payload& payload() _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT void commit() _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _apply_update(IUPayloadUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_link_update(IULinkUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_commission();
		IPAACA_HEADER_EXPORT void _apply_retraction();
	typedef boost::shared_ptr<RemotePushIU> ptr;
};//}}}
/// Copy of a remote Message, received in an InputBuffer. Setter functions all fail.\b Note: Typically handled only as reference in a handler in user space.
IPAACA_HEADER_EXPORT class RemoteMessage: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	friend class MessageConverter;
	public:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
	protected:
		IPAACA_HEADER_EXPORT RemoteMessage();
		IPAACA_HEADER_EXPORT static boost::shared_ptr<RemoteMessage> create();
	public:
		IPAACA_HEADER_EXPORT inline ~RemoteMessage() {
			//IPAACA_IMPLEMENT_ME
		}
		IPAACA_HEADER_EXPORT inline Payload& payload() _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT inline const Payload& const_payload() const _IPAACA_OVERRIDE_ { return _payload; }
		IPAACA_HEADER_EXPORT void commit() _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _apply_update(IUPayloadUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_link_update(IULinkUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_commission();
		IPAACA_HEADER_EXPORT void _apply_retraction();
	typedef boost::shared_ptr<RemoteMessage> ptr;
};//}}}

/// Mock IU for testing purposes. [INTERNAL]
IPAACA_HEADER_EXPORT class FakeIU: public IUInterface {//{{{
	friend class Buffer;
	friend class InputBuffer;
	friend class OutputBuffer;
	friend class IUConverter;
	friend class MessageConverter;
	protected:
		IPAACA_MEMBER_VAR_EXPORT Payload _payload;
		IPAACA_HEADER_EXPORT FakeIU();
	public:
		IPAACA_HEADER_EXPORT static boost::shared_ptr<FakeIU> create();
		IPAACA_HEADER_EXPORT ~FakeIU();
		IPAACA_HEADER_EXPORT Payload& payload() _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT const Payload& const_payload() const _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void commit() _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void add_fake_payload_item(const std::string& key, PayloadDocumentEntry::ptr entry);
	protected:
		IPAACA_HEADER_EXPORT void _modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
		IPAACA_HEADER_EXPORT void _modify_payload(bool is_delta, const std::map<std::string, PayloadDocumentEntry::ptr>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name = "") _IPAACA_OVERRIDE_;
	protected:
		IPAACA_HEADER_EXPORT void _apply_update(IUPayloadUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_link_update(IULinkUpdate::ptr update);
		IPAACA_HEADER_EXPORT void _apply_commission();
		IPAACA_HEADER_EXPORT void _apply_retraction();
	public:
	typedef boost::shared_ptr<FakeIU> ptr;
};//}}}

#endif
