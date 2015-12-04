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

// UUID generation
IPAACA_EXPORT std::string generate_uuid_string()//{{{
{
#ifdef WIN32
	// Windows
	UUID uuid;
	RPC_STATUS stat;
	stat = UuidCreate(&uuid);
	if (stat == RPC_S_OK) {
		unsigned char* uuid_str = NULL;
		stat = UuidToString(&uuid, &uuid_str);
		if (stat == RPC_S_OK) {
			std::string result((const char*) uuid_str, 16);
			RpcStringFree(&uuid_str);
			return result;
		}
	} else {
		throw UUIDGenerationError();
	}
#else
	// POSIX
	uuid_t uuidt;
	uuid_generate(uuidt);
#ifdef __MACOSX__
	//   (Mac)
	uuid_string_t uuidstr;
	uuid_unparse_lower(uuidt, uuidstr);
	return uuidstr;
#else
	//   (Linux)
	char result_c[37];
	uuid_unparse_lower(uuidt, result_c);
	return result_c;
#endif
#endif
}//}}}

IPAACA_EXPORT std::string __ipaaca_static_option_default_payload_type("JSON");
IPAACA_EXPORT std::string __ipaaca_static_option_default_channel("default");
IPAACA_EXPORT unsigned int __ipaaca_static_option_log_level(IPAACA_LOG_LEVEL_WARNING);

} // of namespace ipaaca


