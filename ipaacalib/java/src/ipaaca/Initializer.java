/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2015 Social Cognitive Systems Group
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

package ipaaca;

import java.nio.ByteBuffer;
import ipaaca.protobuf.Ipaaca.IUCommission;
import ipaaca.protobuf.Ipaaca.IUResendRequest;
import ipaaca.protobuf.Ipaaca.IURetraction;
import rsb.converter.ConverterSignature;
import rsb.converter.ConverterRepository;
import rsb.converter.DefaultConverterRepository;
import rsb.converter.ProtocolBufferConverter;

/**
 * Hooks up the ipaaca converters, call initializeIpaacaRsb() before using ipaaca.
 * @author hvanwelbergen
 * 
 */
public final class Initializer {

    private Initializer() {}

    private static volatile boolean initialized = false;
    
    public synchronized static void initializeIpaacaRsb() {
        if (initialized)
            return;

        ConverterRepository<ByteBuffer> dcr =
            DefaultConverterRepository.getDefaultConverterRepository();

        // for IU revision numbers
        dcr.addConverter(
            new IntConverter());

        // IU commit messages
        dcr.addConverter(
            new ProtocolBufferConverter<IUCommission>(
                IUCommission.getDefaultInstance()));

        // IU commit messages
        dcr.addConverter(
            new ProtocolBufferConverter<IURetraction>(
                IURetraction.getDefaultInstance()));

        // IU resend request messages
        dcr.addConverter(
            new ProtocolBufferConverter<IUResendRequest>(
                IUResendRequest.getDefaultInstance()));

        // IUs
        dcr.addConverter(
            new IUConverter(
                new ConverterSignature(
                    "ipaaca-iu",
                    RemotePushIU.class)));

        // Local IUs
        dcr.addConverter(
            new IUConverter(
                new ConverterSignature(
                    "ipaaca-localiu",
                    LocalIU.class)));

        // Messages
        dcr.addConverter(
            new IUConverter(
                new ConverterSignature(
                    "ipaaca-messageiu",
                    RemoteMessageIU.class)));

        // LocalMessages
        dcr.addConverter(
            new IUConverter(
                new ConverterSignature(
                    "ipaaca-localmessageiu",
                    LocalMessageIU.class)));
         
        // Payloads
        dcr.addConverter(
            new PayloadConverter());

        // LinkUpdates
        dcr.addConverter(
            new LinkUpdateConverter());

        initialized = true;
    }
}
