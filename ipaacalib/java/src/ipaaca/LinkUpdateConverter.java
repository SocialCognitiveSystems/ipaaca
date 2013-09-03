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

import ipaaca.protobuf.Ipaaca.IULinkUpdate;

import java.nio.ByteBuffer;

import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;

import com.google.protobuf.InvalidProtocolBufferException;

/**
 * Serializer/deserializer for IULinkUpdate
 * @author hvanwelbergen
 * 
 */
public class LinkUpdateConverter implements Converter<ByteBuffer>
{
    private static final String LINKUPDATE_WIRESCHEMA = "ipaaca-iu-link-update";

    @Override
    public UserData<?> deserialize(String wireSchema, ByteBuffer buffer) throws ConversionException
    {
        IULinkUpdate pl;
        try
        {
            pl = IULinkUpdate.newBuilder().mergeFrom(buffer.array()).build();
        }
        catch (InvalidProtocolBufferException e)
        {
            throw new RuntimeException(e);
        }
        return new UserData<IULinkUpdate>(pl, IULinkUpdate.class);
    }

    @Override
    public ConverterSignature getSignature()
    {
        return new ConverterSignature(LINKUPDATE_WIRESCHEMA, IULinkUpdate.class);
    }

    @Override
    public WireContents<ByteBuffer> serialize(Class<?> typeInfo, Object obj) throws ConversionException
    {
        IULinkUpdate pl = (IULinkUpdate) obj;
        return new WireContents<ByteBuffer>(ByteBuffer.wrap(pl.toByteArray()), LINKUPDATE_WIRESCHEMA);
    }

}
