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
 * \file   ipaaca.h
 *
 * \brief Central header file for IPAACA-C++.
 *
 * This is the central header file for the C++ version of IPAACA. Users should <b>include this file only.</b>
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

/**
\mainpage Documentation for IPAACA-C++

This is the automatically generated documentation for the C++ implementation of IPAACA.

List of most relevant entry points:

Buffers: InputBuffer, OutputBuffer

IUs: IUInterface, IU, Message

IU handling (user-set): #IUEventHandlerFunction

IU payload contents: Payload, PayloadEntryProxy
*/

#ifndef __ipaaca_h_INCLUDED__
#define __ipaaca_h_INCLUDED__

/// ipaaca/IU/RSB protocol major version number
#define IPAACA_PROTOCOL_VERSION_MAJOR         2
/// ipaaca/IU/RSB protocol minor version number
#define IPAACA_PROTOCOL_VERSION_MINOR         0

/// running release number of ipaaca-c++
#define IPAACA_CPP_RELEASE_NUMBER             13
/// date of last release number increment
#define IPAACA_CPP_RELEASE_DATE     "2015-12-04"

#ifndef __FUNCTION_NAME__
	#ifdef WIN32   // Windows
		#define __FUNCTION_NAME__   __FUNCTION__
	#else          // POSIX
		#define __FUNCTION_NAME__   __func__
	#endif
#endif

#ifdef WIN32
	#define IPAACA_SYSTEM_DEPENDENT_CLASS_NAME(c) "class "##c
#else
	#define IPAACA_SYSTEM_DEPENDENT_CLASS_NAME(c) c
#endif

#ifdef WIN32
	#if defined(ipaaca_EXPORTS)
		#define  IPAACA_EXPORT
		#define  IPAACA_HEADER_EXPORT __declspec(dllexport)
		#define  IPAACA_MEMBER_VAR_EXPORT
	#else
		#define  IPAACA_EXPORT
		#define  IPAACA_HEADER_EXPORT __declspec(dllimport)
		#define  IPAACA_MEMBER_VAR_EXPORT
	#endif
#else
	#define IPAACA_EXPORT
	#define IPAACA_HEADER_EXPORT
	#define IPAACA_MEMBER_VAR_EXPORT
#endif

#ifdef IPAACA_DEBUG_MESSAGES
#define IPAACA_DEBUG(i)     if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_DEBUG) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- Debug: " << i << std::endl; }
#define IPAACA_INFO(i)      if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_INFO) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- Info: " << i << std::endl; }
#define IPAACA_WARNING(i)   if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_WARNING) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- WARNING: " << i << std::endl; }
#define IPAACA_ERROR(i)     if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_ERROR) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- ERROR: " << i << std::endl; }
#define IPAACA_CRITICAL(i)  if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_CRITICAL) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- CRITICAL: " << i << std::endl; }
#define IPAACA_IMPLEMENT_ME if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_INFO) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- IMPLEMENT_ME:" << std::endl; }
#define IPAACA_TODO(i)      if (__ipaaca_static_option_log_level>=IPAACA_LOG_LEVEL_INFO) { std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION_NAME__ << "() -- TODO: " << i << std::endl; }
#else
#define IPAACA_DEBUG(i) ;
#define IPAACA_INFO(i) ;
#define IPAACA_WARNING(i) ;
#define IPAACA_ERROR(i) ;
#define IPAACA_CRITICAL(i) ;
#define IPAACA_IMPLEMENT_ME ;
#define IPAACA_TODO(i) ;
#endif

#ifdef IPAACA_EXPOSE_FULL_RSB_API
#include <rsc/runtime/TypeStringTools.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Event.h>
#include <rsb/ParticipantConfig.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>
#include <rsb/converter/Converter.h>
#include <rsb/rsbexports.h>
#endif

// new json-based payload API, used in several classes
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"
#include <cstdio>

/// marking pure virtual functions for extra readability
#define _IPAACA_ABSTRACT_

#define _IPAACA_OVERRIDE_ override

/// value to return when reading nonexistant payload keys
#define IPAACA_PAYLOAD_DEFAULT_STRING_VALUE ""

// seconds until remote writes time out
#define IPAACA_REMOTE_SERVER_TIMEOUT 2.0


#include <iostream>

// for logger
#include <iomanip>

#ifdef WIN32
// for Windows
#include <time.h>
#else
// for Linux and OS X
#include <sys/time.h>
#endif

#include <cstdlib>


#ifdef WIN32
#include <rpc.h>
#else
#include <uuid/uuid.h>
#include <glob.h>
#endif


#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/lexical_cast.hpp>
#endif


#include <ipaaca/ipaaca.pb.h>

#include <set>
#include <list>
#include <algorithm>
#include <utility>
#include <initializer_list>

namespace ipaaca {

#include <ipaaca/ipaaca-definitions.h>
#include <ipaaca/ipaaca-forwards.h>

#include <ipaaca/ipaaca-locking.h>

// Global static library variables (run-time default configuration)
// Actual initial values are set in ipaaca.cc

/// Default payload type for new IUs (defaults to "JSON")
IPAACA_MEMBER_VAR_EXPORT extern std::string __ipaaca_static_option_default_payload_type;
/// Default channel for buffers (defaults to "default")
IPAACA_MEMBER_VAR_EXPORT extern std::string __ipaaca_static_option_default_channel;
/// Current console log level (defaults to warning), one of: IPAACA_LOG_LEVEL_CRITICAL, IPAACA_LOG_LEVEL_ERROR, IPAACA_LOG_LEVEL_WARNING, IPAACA_LOG_LEVEL_INFO, IPAACA_LOG_LEVEL_DEBUG, IPAACA_LOG_LEVEL_NONE
IPAACA_MEMBER_VAR_EXPORT extern unsigned int __ipaaca_static_option_log_level;

/// RSB host to connect to (defaults to "" = do not set, use global config)
IPAACA_MEMBER_VAR_EXPORT extern std::string __ipaaca_static_option_rsb_spread_host;
/// RSB port to connect to (defaults to "" = do not set, use global config)
IPAACA_MEMBER_VAR_EXPORT extern std::string __ipaaca_static_option_rsb_spread_port;

IPAACA_MEMBER_VAR_EXPORT Lock& logger_lock();

#ifdef WIN32
#define LOG_IPAACA_CONSOLE(msg) { ipaaca::Locker logging_locker(ipaaca::logger_lock()); std::time_t result = std::time(NULL); std::cout << "[LOG] " << std::asctime(std::localtime(&result)) << " : " << msg << std::endl; }
#else
// use normal gettimeofday() on POSIX
#define LOG_IPAACA_CONSOLE(msg) { ipaaca::Locker logging_locker(ipaaca::logger_lock()); timeval logging_tim; gettimeofday(&logging_tim, NULL); double logging_t1=logging_tim.tv_sec+(logging_tim.tv_usec/1000000.0); std::cout << "[LOG] " << std::setprecision(15) << logging_t1 << " : " << msg << std::endl; }
#endif

#include <ipaaca/ipaaca-payload.h>
#include <ipaaca/ipaaca-buffers.h>
#include <ipaaca/ipaaca-ius.h>

/// Full API (including RSB transport) is only exposed during
///  ipaaca compilation, user programs should use abstract API.
#ifdef IPAACA_EXPOSE_FULL_RSB_API
#include <ipaaca/ipaaca-internal.h>
#endif



} // of namespace ipaaca

#endif


