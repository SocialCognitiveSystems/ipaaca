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

package ipaaca;

import ipaaca.protobuf.Ipaaca.IUCommission;
import rsb.converter.ConverterSignature;
import rsb.converter.DefaultConverterRepository;
import rsb.converter.ProtocolBufferConverter;

/**
 * Hooks up the ipaaca converters, call initializeIpaacaRsb() before using ipaaca.
 * @author hvanwelbergen
 * 
 */
public final class Initializer
{
    private Initializer()
    {
    }
    private static volatile boolean initialized = false;
    
    public synchronized static void initializeIpaacaRsb()
    {
        if(initialized)return;
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new IntConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new ProtocolBufferConverter<IUCommission>(IUCommission.getDefaultInstance()));

        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-iu", RemotePushIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
         new IUConverter(new ConverterSignature("ipaaca-localiu", LocalIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-messageiu", RemoteMessageIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-localmessageiu", LocalMessageIU.class)));
         
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new PayloadConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new LinkUpdateConverter());
        initialized = true;
    }
}
