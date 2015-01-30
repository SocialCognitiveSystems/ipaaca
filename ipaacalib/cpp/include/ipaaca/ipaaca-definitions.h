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

#ifndef __ipaaca_definitions_h_INCLUDED__
#define __ipaaca_definitions_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif


typedef uint32_t revision_t;

/// Type of the IU event. Realized as an integer to enable bit masks for filters.
typedef uint32_t IUEventType;
#define IU_ADDED         1
#define IU_COMMITTED     2
#define IU_DELETED       4
#define IU_RETRACTED     8
#define IU_UPDATED      16
#define IU_LINKSUPDATED 32
#define IU_MESSAGE      64
/// Bit mask for receiving all events
#define IU_ALL_EVENTS  127

/// Convert an int event type to a human-readable string
IPAACA_HEADER_EXPORT inline std::string iu_event_type_to_str(IUEventType type)
{
	switch(type) {
		case IU_ADDED: return "ADDED";
		case IU_COMMITTED: return "COMMITTED";
		case IU_DELETED: return "DELETED";
		case IU_RETRACTED: return "RETRACTED";
		case IU_UPDATED: return "UPDATED";
		case IU_LINKSUPDATED: return "LINKSUPDATED";
		case IU_MESSAGE: return "MESSAGE";
		default: return "(NOT A KNOWN SINGLE IU EVENT TYPE)";
	}
}

/// IU access mode: PUSH means that updates are broadcast; REMOTE means that reads are RPC calls; MESSAGE means a fire-and-forget message
IPAACA_HEADER_EXPORT enum IUAccessMode {
	IU_ACCESS_PUSH,
	IU_ACCESS_REMOTE,
	IU_ACCESS_MESSAGE
};

/// generate a UUID as an ASCII string
IPAACA_HEADER_EXPORT std::string generate_uuid_string();

/*
 *  Exceptions and errors
 */
IPAACA_HEADER_EXPORT class Exception: public std::exception//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _description;
	public:
		IPAACA_HEADER_EXPORT inline Exception(const std::string& description=""): _description(description) { }
		IPAACA_HEADER_EXPORT inline ~Exception() throw() { }
		IPAACA_HEADER_EXPORT const char* what() const throw() {
			return _description.c_str();
		}
};//}}}
IPAACA_HEADER_EXPORT class Abort: public std::exception//{{{
{
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::string _description;
	public:
		IPAACA_HEADER_EXPORT inline Abort(const std::string& description=""): _description(description) { }
		IPAACA_HEADER_EXPORT inline ~Abort() throw() { }
		IPAACA_HEADER_EXPORT const char* what() const throw() {
			return _description.c_str();
		}
};//}}}

IPAACA_HEADER_EXPORT class IUNotFoundError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUNotFoundError() throw() { }
		IPAACA_HEADER_EXPORT inline IUNotFoundError() { //boost::shared_ptr<IU> iu) {
			_description = "IUNotFoundError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUPublishedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUPublishedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUPublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUPublishedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUCommittedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUCommittedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUCommittedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUCommittedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUUpdateFailedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUUpdateFailedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUUpdateFailedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUUpdateFailedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUResendRequestFailedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUResendRequestFailedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUResendRequestFailedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUResendRequestFailedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUReadOnlyError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUReadOnlyError() throw() { }
		IPAACA_HEADER_EXPORT inline IUReadOnlyError() { //boost::shared_ptr<IU> iu) {
			_description = "IUReadOnlyError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyInABufferError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyInABufferError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyInABufferError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyInABufferError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUUnpublishedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUUnpublishedError() throw() { }
		IPAACA_HEADER_EXPORT inline IUUnpublishedError() { //boost::shared_ptr<IU> iu) {
			_description = "IUUnpublishedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyHasAnUIDError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyHasAnUIDError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyHasAnUIDError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnUIDError";
		}
};//}}}
IPAACA_HEADER_EXPORT class IUAlreadyHasAnOwnerNameError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~IUAlreadyHasAnOwnerNameError() throw() { }
		IPAACA_HEADER_EXPORT inline IUAlreadyHasAnOwnerNameError() { //boost::shared_ptr<IU> iu) {
			_description = "IUAlreadyHasAnOwnerNameError";
		}
};//}}}
IPAACA_HEADER_EXPORT class UUIDGenerationError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~UUIDGenerationError() throw() { }
		IPAACA_HEADER_EXPORT inline UUIDGenerationError() { //boost::shared_ptr<IU> iu) {
			_description = "UUIDGenerationError";
		}
};//}}}
IPAACA_HEADER_EXPORT class NotImplementedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~NotImplementedError() throw() { }
		IPAACA_HEADER_EXPORT inline NotImplementedError() { //boost::shared_ptr<IU> iu) {
			_description = "NotImplementedError";
		}
};//}}}


/// Static library initialization
IPAACA_HEADER_EXPORT class Initializer
{
	public:
		IPAACA_HEADER_EXPORT static void initialize_ipaaca_rsb_if_needed();
		IPAACA_HEADER_EXPORT static void initialize_updated_default_config();
		IPAACA_HEADER_EXPORT static bool initialized();
		IPAACA_HEADER_EXPORT static void dump_current_default_config();
	protected:
		IPAACA_MEMBER_VAR_EXPORT static bool _initialized;
};

// additional misc classes ( Command line options )//{{{
IPAACA_HEADER_EXPORT class CommandLineOptions {
	public:
		IPAACA_HEADER_EXPORT inline CommandLineOptions() { }
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> param_opts;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> param_set;
	public:
		IPAACA_HEADER_EXPORT void set_option(const std::string& name, bool expect, const char* optarg);
		IPAACA_HEADER_EXPORT std::string get_param(const std::string& o);
		IPAACA_HEADER_EXPORT bool is_set(const std::string& o);
		IPAACA_HEADER_EXPORT void dump();
	typedef boost::shared_ptr<CommandLineOptions> ptr;
};

class CommandLineParser {
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::map<char, std::string> longopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, char> shortopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> options; //  name / expect_param
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> defaults; // for opt params
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, int> set_flag; // for paramless opts
	protected:
		IPAACA_HEADER_EXPORT CommandLineParser();
	public:
		IPAACA_HEADER_EXPORT inline ~CommandLineParser() { }
		IPAACA_HEADER_EXPORT static inline boost::shared_ptr<CommandLineParser> create() {
			return boost::shared_ptr<CommandLineParser>(new CommandLineParser());
		}
		IPAACA_HEADER_EXPORT void initialize_parser_defaults();
		IPAACA_HEADER_EXPORT void dump_options();
		IPAACA_HEADER_EXPORT void add_option(const std::string& optname, char shortn, bool expect_param, const std::string& defaultv);
		IPAACA_HEADER_EXPORT void ensure_defaults_in( CommandLineOptions::ptr clo );
		IPAACA_HEADER_EXPORT CommandLineOptions::ptr parse(int argc, char* const* argv);
	typedef boost::shared_ptr<CommandLineParser> ptr;
};
//}}}
// additional misc functions ( String splitting / joining )//{{{
IPAACA_HEADER_EXPORT std::string str_join(const std::set<std::string>& set,const std::string& sep);
IPAACA_HEADER_EXPORT std::string str_join(const std::vector<std::string>& vec,const std::string& sep);
IPAACA_HEADER_EXPORT void str_split_wipe(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
IPAACA_HEADER_EXPORT void str_split_append(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
//}}}

#endif
