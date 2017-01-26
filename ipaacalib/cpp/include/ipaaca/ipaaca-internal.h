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
 * \file   ipaaca-internal.h
 *
 * \brief Header file for internal transport implementation (RSB).
 *
 * Users should not include this file directly, but use ipaaca.h
 *
 * \b Note: This file is only included during compilation of ipaaca,
 * for regular use, the full internal API is not exposed.
 * Users generally need never touch the internal transport layer.
 *
 * The file provides callback glue from RSB, and wire converters
 * for the respective ipaaca classes.
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#ifndef __ipaaca_internal_h_INCLUDED__
#define __ipaaca_internal_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif


#ifdef IPAACA_EXPOSE_FULL_RSB_API

IPAACA_HEADER_EXPORT class CallbackIUPayloadUpdate: public rsb::patterns::LocalServer::Callback<IUPayloadUpdate, int64_t> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUPayloadUpdate(Buffer* buffer);
		IPAACA_HEADER_EXPORT boost::shared_ptr<int64_t> call(const std::string& methodName, boost::shared_ptr<IUPayloadUpdate> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIULinkUpdate: public rsb::patterns::LocalServer::Callback<IULinkUpdate, int64_t> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIULinkUpdate(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int64_t> call(const std::string& methodName, boost::shared_ptr<IULinkUpdate> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIUCommission: public rsb::patterns::LocalServer::Callback<protobuf::IUCommission, int64_t> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUCommission(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int64_t> call(const std::string& methodName, boost::shared_ptr<protobuf::IUCommission> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIUResendRequest: public rsb::patterns::LocalServer::Callback<protobuf::IUResendRequest, int64_t> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIUResendRequest(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int64_t> call(const std::string& methodName, boost::shared_ptr<protobuf::IUResendRequest> update);
};//}}}
IPAACA_HEADER_EXPORT class CallbackIURetraction: public rsb::patterns::LocalServer::Callback<protobuf::IURetraction, int64_t> {//{{{
	protected:
		IPAACA_MEMBER_VAR_EXPORT Buffer* _buffer;
	public:
		IPAACA_HEADER_EXPORT CallbackIURetraction(Buffer* buffer);
	public:
		IPAACA_HEADER_EXPORT boost::shared_ptr<int64_t> call(const std::string& methodName, boost::shared_ptr<protobuf::IURetraction> update);
};//}}}

IPAACA_HEADER_EXPORT class IUConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IUConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class MessageConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT MessageConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class IUPayloadUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IUPayloadUpdateConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
IPAACA_HEADER_EXPORT class IULinkUpdateConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IULinkUpdateConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
/*
IPAACA_HEADER_EXPORT class IntConverter: public rsb::converter::Converter<std::string> {//{{{
	public:
		IPAACA_HEADER_EXPORT IntConverter();
		IPAACA_HEADER_EXPORT std::string serialize(const rsb::AnnotatedData& data, std::string& wire);
		IPAACA_HEADER_EXPORT rsb::AnnotatedData deserialize(const std::string& wireSchema, const std::string& wire);
};//}}}
*/
#endif

#endif
