/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2013 Sociable Agents Group
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

#ifndef WIN32
#include <getopt.h>
#endif

namespace ipaaca {

//
// Command line options implementation
//


void CommandLineOptions::set_option(const std::string& name, bool expect, const char* optarg) {
	param_set[name] = true;
	if (expect) {
		param_opts[name] = optarg;
	}
}
std::string CommandLineOptions::get_param(const std::string& o) {
	std::map<std::string, std::string>::iterator it = param_opts.find(o);
	if (it==param_opts.end()) return "";
	return it->second;
}
bool CommandLineOptions::is_set(const std::string& o) {
	std::map<std::string, bool>::iterator it = param_set.find(o);
	if (it==param_set.end()) return false;
	return it->second;
}
void CommandLineOptions::dump() {
	for (std::map<std::string, bool>::iterator it=param_set.begin(); it!=param_set.end(); ++it) {
		std::map<std::string, std::string>::iterator it2 = param_opts.find( it->first );
		if (it2 == param_opts.end()) {
			std::cout << it->first << "\t<true>" << std::endl;
		} else {
			std::cout << it->first << "\t" << it2->second << std::endl;
		}
	}
}

//
// Command line parser implementation
//

CommandLineParser::CommandLineParser()
: library_options_handled(true)
{
	initialize_parser_defaults();
}

void CommandLineParser::initialize_parser_defaults()
{
	add_option("help",           0  , false, "");
	add_option("verbose",        'v', false, "");
	add_option("character-name", 'c', true, "UnknownCharacter");
	add_option("component-name", 'n', true, "UnknownComponent");
	if (library_options_handled) {
		add_option("ipaaca-payload-type", 0, true, "JSON");
		add_option("ipaaca-default-channel", 0, true, "default");
		add_option("ipaaca-enable-logging", 0, true, "WARNING");
		add_option("rsb-enable-logging", 0, true, "ERROR");
	}
}

bool CommandLineParser::consume_library_option(const std::string& name, bool expect, const char* optarg)
{
	if (name=="ipaaca-payload-type") {
		std::string newtype = optarg;
		if (newtype=="MAP") newtype="STR";
		if ((newtype=="JSON") || (newtype=="STR")) {
			IPAACA_DEBUG("Setting default payload type " << newtype)
			__ipaaca_static_option_default_payload_type = newtype;
		} else {
			IPAACA_WARNING("Ignoring unknown default payload type " << newtype << " - should be one of JSON or STR")
		}
	} else if (name=="ipaaca-default-channel") {
		std::string newch = optarg;
		IPAACA_DEBUG("Setting default channel " << newch)
		__ipaaca_static_option_default_channel = newch;
	} else if (name=="ipaaca-enable-logging") {
		std::string level(optarg);
		if ((level=="NONE") || (level=="SILENT")) {
			IPAACA_DEBUG("Will set log level to NONE")
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_NONE;
		} else if (level=="DEBUG") {
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_DEBUG;
			IPAACA_DEBUG("Just set log level to DEBUG")
		} else if (level=="INFO") {
			IPAACA_DEBUG("Set log level to INFO")
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_INFO;
		} else if (level=="WARNING") {
			IPAACA_DEBUG("Set log level to WARNING")
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_WARNING;
		} else if (level=="ERROR") {
			IPAACA_DEBUG("Set log level to ERROR")
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_ERROR;
		} else if (level=="CRITICAL") {
			IPAACA_DEBUG("Set log level to CRITICAL")
			__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_CRITICAL;
		} else {
			IPAACA_WARNING("Unknown log level " << optarg)
			IPAACA_WARNING("Valid levels are: NONE, DEBUG, INFO, WARNING, ERROR, CRITICAL ")
		}
	} else if (name=="rsb-enable-logging") {
		IPAACA_WARNING("Unimplemented option ignored: " << name)
		IPAACA_IMPLEMENT_ME
	} else {
		return false;
	}
	return true;
}

void CommandLineParser::dump_options()
{
	for (std::map<std::string, bool>::const_iterator it = options.begin(); it!=options.end(); ++it) {
		const std::string& optn = it->first;
		bool expect = it->second;
		char shortn = shortopt[optn];
		std::string shortns; shortns += shortn;
		if (optn != "help") {
			if (shortn) std::cout << "    -" << shortns << " | --" << optn << " ";
			else std::cout << "    --" << optn << " ";
			if (expect) {
				std::cout << "<param>";
				std::cout << " (default: '" << defaults[optn] << "')";
			}
			std::cout << std::endl;
		}
	}
}

void CommandLineParser::add_option(const std::string& optname, char shortoptn, bool expect_param, const std::string& defaultv)
{
	longopt[shortoptn] = optname;
	shortopt[optname] = shortoptn;
	options[optname] = expect_param;
	defaults[optname] = defaultv;
	set_flag[optname] = 0;
}

CommandLineOptions::ptr CommandLineParser::parse(int argc, char* const* argv)
{
#ifdef WIN32
	LOG_IPAACA_CONSOLE("IMPLEMENT ME: command line parsing for Windows. (req'd: getopt)")
	throw NotImplementedError();
#else
	IPAACA_DEBUG("")
	int len = options.size();
	struct option long_options[len+1];
	int i=0;
	std::string short_options_str = "";
	for (std::map<std::string, bool>::const_iterator it = options.begin(); it!=options.end(); ++it) {
		const std::string& optn = it->first;
		bool expect = it->second;
		char shortn = shortopt[optn];
		int* write_to = &(set_flag[optn]);
		if (shortn > ' ') {
			short_options_str += shortn;
			if (expect) short_options_str += ':';
		}
		long_options[i].name = optn.c_str();
		long_options[i].has_arg = (expect?required_argument:no_argument);
		long_options[i].flag = (expect?write_to:0);
		long_options[i].val = shortn;
		i++;
	}
	long_options[i].name = 0;
	long_options[i].has_arg = 0;
	long_options[i].flag = 0;
	long_options[i].val = 0;
	CommandLineOptions::ptr clo = CommandLineOptions::ptr(new CommandLineOptions());
	int c;
	bool keep_going = true;
	while (keep_going)
	{
		// getopt_long stores the option index here.
		int option_index = 0;
		c = getopt_long (argc, argv, short_options_str.c_str(), long_options, &option_index);
		// Detect the end of the options. 
		if (c == -1) break;
		
		bool do_set_option = false;
		std::string longname;
		std::string longoption;
		bool expect;
		switch (c)
		{
			case 0:
				{
				longname = long_options[option_index].name;
				if (longname == "help") {
					std::cout << "Options:" << std::endl;
					dump_options();
					exit(0);
				}
				longoption = long_options[option_index].name;
				expect = options[longoption];
				do_set_option = true;
				}
				break;

			case '?':
				break;

			default:
				std::string s;
				s += c;
				longoption = longopt[c];
				expect = options[longoption];
				do_set_option = true;
		}
		if (do_set_option) {
			if (library_options_handled) {
				do_set_option = ! consume_library_option(longoption, expect, optarg );
			}
			if (do_set_option) {
				clo->set_option(longoption, expect, optarg);
			}
		}
	}
	ensure_defaults_in( clo );
	return clo;
#endif
}

void CommandLineParser::ensure_defaults_in( CommandLineOptions::ptr clo )
{
	for (std::map<std::string, bool>::const_iterator it = options.begin(); it!=options.end(); ++it) {
		const std::string& optn = it->first;
		bool expect = it->second;
		char shortn = shortopt[optn];
		if (expect && (! clo->is_set(optn))) {
			clo->set_option(optn, true, defaults[optn].c_str());
		}
	}
}

} // namespace ipaaca

