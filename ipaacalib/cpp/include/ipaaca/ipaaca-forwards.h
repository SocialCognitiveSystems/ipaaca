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
 * \file   ipaaca-forwards.h
 *
 * \brief Header file for forward definitions.
 *
 * Users should not include this file directly, but use ipaaca.h
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#ifndef __ipaaca_forwards_h_INCLUDED__
#define __ipaaca_forwards_h_INCLUDED__

#ifndef __ipaaca_h_INCLUDED__
#error "Please do not include this file directly, use ipaaca.h instead"
#endif

/*
 *  forward declarations
 */
class PayloadDocumentEntry;

class PayloadBatchUpdateLock;
class PayloadEntryProxy;
class Payload;
class PayloadIterator;
class IUInterface;
class IU;
class Message;
class RemotePushIU;
class IULinkUpdate;
class IUPayloadUpdate;
class IUStore;
class FrozenIUStore;
class Buffer;
class InputBuffer;
class OutputBuffer;

class CallbackIUPayloadUpdate;
class CallbackIULinkUpdate;
class CallbackIUCommission;
class CallbackIUResendRequest;
class CallbackIURetraction;

class IUConverter;
class MessageConverter;
class IUPayloadUpdateConverter;
class IULinkUpdateConverter;
//class IntConverter;

class BufferConfiguration;
class BufferConfigurationBuilder;

#endif

