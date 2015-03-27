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
 * \file   ipaaca-definitions.h
 *
 * \brief Header file for data and exception types and helper functions.
 *
 * Users should not include this file directly, but use ipaaca.h
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#ifndef __ipaaca_definitions_h_INCLUDED__
#define __ipaaca_definitions_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif

// LAST FIXME LAST
//typedef boost::shared_ptr<rapidjson::Document> JsonDocPtr;

typedef uint32_t revision_t;

/// Type of the IU event. Realized as an integer to enable bit masks for filters. One of: IU_ADDED, IU_COMMITTED, IU_DELETED, IU_RETRACTED, IU_UPDATED, IU_LINKSUPDATED, IU_MESSAGE
typedef uint32_t IUEventType;
#define IU_ADDED         1
#define IU_COMMITTED     2
#define IU_DELETED       4
#define IU_RETRACTED     8
#define IU_UPDATED      16
#define IU_LINKSUPDATED 32
#define IU_MESSAGE      64
/// Bit mask for receiving all IU events  \see IUEventType
#define IU_ALL_EVENTS  127

/// Ipaaca (console) log levels
#define IPAACA_LOG_LEVEL_NONE     0
#define IPAACA_LOG_LEVEL_CRITICAL 1
#define IPAACA_LOG_LEVEL_ERROR    2
#define IPAACA_LOG_LEVEL_WARNING  3
#define IPAACA_LOG_LEVEL_INFO     4
#define IPAACA_LOG_LEVEL_DEBUG    5

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

/// IU access mode: PUSH means that updates are broadcast; REMOTE means that reads are RPC calls (currently NOT implemented); MESSAGE means a fire-and-forget message
IPAACA_HEADER_EXPORT enum IUAccessMode {
	IU_ACCESS_PUSH,
	IU_ACCESS_REMOTE,
	IU_ACCESS_MESSAGE
};

/// generate a UUID as an ASCII string
IPAACA_HEADER_EXPORT std::string generate_uuid_string();

/**
 * Exception with string description
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
IPAACA_HEADER_EXPORT class PayloadTypeConversionError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~PayloadTypeConversionError() throw() { }
		IPAACA_HEADER_EXPORT inline PayloadTypeConversionError() { //boost::shared_ptr<IU> iu) {
			_description = "PayloadTypeConversionError";
		}
};//}}}
IPAACA_HEADER_EXPORT class PayloadAddressingError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~PayloadAddressingError() throw() { }
		IPAACA_HEADER_EXPORT inline PayloadAddressingError() { //boost::shared_ptr<IU> iu) {
			_description = "PayloadAddressingError";
		}
};//}}}
IPAACA_HEADER_EXPORT class JsonParsingError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~JsonParsingError() throw() { }
		IPAACA_HEADER_EXPORT inline JsonParsingError() { //boost::shared_ptr<IU> iu) {
			_description = "JsonParsingError";
		}
};//}}}
IPAACA_HEADER_EXPORT class PayloadEntryProxyInvalidatedError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~PayloadEntryProxyInvalidatedError() throw() { }
		IPAACA_HEADER_EXPORT inline PayloadEntryProxyInvalidatedError() { //boost::shared_ptr<IU> iu) {
			_description = "PayloadEntryProxyInvalidatedError";
		}
};//}}}
IPAACA_HEADER_EXPORT class PayloadIteratorInvalidError: public Exception//{{{
{
	public:
		IPAACA_HEADER_EXPORT inline ~PayloadIteratorInvalidError() throw() { }
		IPAACA_HEADER_EXPORT inline PayloadIteratorInvalidError() { //boost::shared_ptr<IU> iu) {
			_description = "PayloadIteratorInvalidError";
		}
};//}}}

/** \brief Static library initialization for backend
 *
 * This static class (singleton) is called once (explicitly or on-demand).
 * Unless called manually, it is initialized when ipaaca is first used
 * (i.e. the first Buffer is created).
 */
IPAACA_HEADER_EXPORT class Initializer
{
	public:
		/// Explicitly initialize the backend.
		IPAACA_HEADER_EXPORT static void initialize_ipaaca_rsb_if_needed();
		IPAACA_HEADER_EXPORT static void initialize_updated_default_config();
		IPAACA_HEADER_EXPORT static bool initialized();
		IPAACA_HEADER_EXPORT static void dump_current_default_config();
	protected:
		IPAACA_MEMBER_VAR_EXPORT static bool _initialized;
};

// in ipaaca-cmdline-parser.cc
// additional misc classes ( Command line options )//{{{
/** \brief Command line argument container for CommandLineParser
 *
 * Contains the results of argument parsing from CommandLineParser::parse()
 *
 * The parser is preconfigured to handle some standard options:
 *
 * Option                          | Function
 * --------------------------------|------------------------------------------------------------------------------
 * --help                          | Print list of available options
 * --verbose                       | Set verbose flag
 * --character-name <name>         | Set character name (legacy)
 * --component-name <name>         | Set component name (legacy)
 * --ipaaca-payload-type <type>    | Set default ipaaca payload type (default JSON, set STR for legacy protocol)
 * --ipaaca-default-channel <name> | Set default channel name (default 'default')
 * --ipaaca-enable-logging <level> | Set console log level, one of NONE, DEBUG, INFO, WARNING, ERROR, CRITICAL
 * --rsb-enable-logging <level>    | Set rsb (transport) log level
 *
 */
IPAACA_HEADER_EXPORT class CommandLineOptions {
	public:
		IPAACA_HEADER_EXPORT inline CommandLineOptions()
		//: _unconsumed_argc(0), _unconsumed_argv(nullptr)
		{ }
		IPAACA_HEADER_EXPORT inline ~CommandLineOptions() {
			//if (_unconsumed_argv) delete[] _unconsumed_argv;
		}
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> param_opts;
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> param_set;
	public:
		IPAACA_HEADER_EXPORT void set_option(const std::string& name, bool expect, const char* optarg);
		/// Get the option argument or default value (if the option expected an argument)
		IPAACA_HEADER_EXPORT std::string get_param(const std::string& o);
		/// Check whether option has been set
		IPAACA_HEADER_EXPORT bool is_set(const std::string& o);
		IPAACA_HEADER_EXPORT void dump();
	public:
		//IPAACA_HEADER_EXPORT inline int unconsumed_argc() { return _unconsumed_argc; }
		//IPAACA_HEADER_EXPORT inline char** unconsumed_argv() { return _unconsumed_argv; }
	protected:
		//IPAACA_MEMBER_VAR_EXPORT int _unconsumed_argc;
		//IPAACA_MEMBER_VAR_EXPORT char** _unconsumed_argv;
	public:
	typedef boost::shared_ptr<CommandLineOptions> ptr;
};

/**
 * \brief Command line parser for ipaaca programs.
 *
 * The parser is preconfigured to handle some standard options:
 *
 * Option                          | Function
 * --------------------------------|------------------------------------------------------------------------------
 * --help                          | Print list of available options
 * --verbose                       | Set verbose flag
 * --character-name <name>         | Set character name (legacy)
 * --component-name <name>         | Set component name (legacy)
 * --ipaaca-payload-type <type>    | Set default ipaaca payload type (default JSON, set STR for legacy protocol)
 * --ipaaca-default-channel <name> | Set default channel name (default 'default')
 * --ipaaca-enable-logging <level> | Set console log level, one of NONE, DEBUG, INFO, WARNING, ERROR, CRITICAL
 * --rsb-enable-logging <level>    | Set rsb (transport) log level
 *
 */
class CommandLineParser {
	protected:
		IPAACA_MEMBER_VAR_EXPORT std::map<char, std::string> longopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, char> shortopt; // letter->name
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, bool> options; //  name / expect_param
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, std::string> defaults; // for opt params
		IPAACA_MEMBER_VAR_EXPORT std::map<std::string, int> set_flag; // for paramless opts
	protected:
		IPAACA_HEADER_EXPORT CommandLineParser();
		IPAACA_MEMBER_VAR_EXPORT bool library_options_handled;
		IPAACA_HEADER_EXPORT bool consume_library_option(const std::string& name, bool expect, const char* optarg);
	public:
		IPAACA_HEADER_EXPORT inline ~CommandLineParser() { }
		/// Create a new parser object reference.
		IPAACA_HEADER_EXPORT static inline boost::shared_ptr<CommandLineParser> create() {
			return boost::shared_ptr<CommandLineParser>(new CommandLineParser());
		}
		IPAACA_HEADER_EXPORT void initialize_parser_defaults();
		IPAACA_HEADER_EXPORT void dump_options();
		/** \brief Add a user-defined option
		 *
		 * \param optname      The long option name, e.g. verbose for --verbose
		 * \param shortn       The short option (or \0 for none)
		 * \param expect_param Whether an argument is expected for the option
		 * \param defaultv     The default string value (unused if expect_param is false)
		 */
		IPAACA_HEADER_EXPORT void add_option(const std::string& optname, char shortn, bool expect_param, const std::string& defaultv);
		IPAACA_HEADER_EXPORT void ensure_defaults_in( CommandLineOptions::ptr clo );
		/** \brief Parse argument list and return result.
		 *
		 * Parse argument list (e.g. from main()) with the parser, consuming the internal options.
		 * The remaining options are packaged into a CommandLineOptions object.
		 */
		IPAACA_HEADER_EXPORT CommandLineOptions::ptr parse(int argc, char* const* argv);
	typedef boost::shared_ptr<CommandLineParser> ptr;
};
//}}}
// in ipaaca-string-utils.cc
// additional misc functions ( String splitting / joining )//{{{
IPAACA_HEADER_EXPORT std::string str_trim(const std::string &s);
IPAACA_HEADER_EXPORT std::string str_join(const std::set<std::string>& set,const std::string& sep);
IPAACA_HEADER_EXPORT std::string str_join(const std::vector<std::string>& vec,const std::string& sep);
IPAACA_HEADER_EXPORT void str_split_wipe(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
IPAACA_HEADER_EXPORT void str_split_append(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters );
//}}}

#endif
