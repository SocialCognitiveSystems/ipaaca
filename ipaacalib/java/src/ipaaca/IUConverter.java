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

import ipaaca.protobuf.Ipaaca.IU;
import ipaaca.protobuf.Ipaaca.LinkSet;
import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;

import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.SetMultimap;
import com.google.protobuf.InvalidProtocolBufferException;

/**
 * Serializes AbstractIUs into protocolbuffer IUs and vice versa.
 * @author hvanwelbergen
 *
 */
public class IUConverter implements Converter<ByteBuffer>
{
    private final ConverterSignature signature;

    public IUConverter(ConverterSignature signature)
    {
        this.signature = signature;
    }

    @Override
    public ConverterSignature getSignature()
    {
        return signature;
    }

    @Override
    public WireContents<ByteBuffer> serialize(Class<?> typeInfo, Object obj) throws ConversionException
    {
        AbstractIU iua = (AbstractIU) obj;
        List<PayloadItem> payloadItems = new ArrayList<PayloadItem>();
        for (Entry<String, String> entry : iua.getPayload().entrySet())
        {
            payloadItems.add(PayloadItem.newBuilder().setKey(entry.getKey()).setValue(entry.getValue()).setType("STR").build());
        }

        List<LinkSet> links = new ArrayList<LinkSet>();
        for (Entry<String, Collection<String>> entry : iua.getAllLinks().asMap().entrySet())
        {
            links.add(LinkSet.newBuilder().setType(entry.getKey()).addAllTargets(entry.getValue()).build());
        }

        IU.AccessMode accessMode = IU.AccessMode.PUSH;
        if(iua instanceof RemoteMessageIU || iua instanceof LocalMessageIU)
        {
            accessMode = IU.AccessMode.MESSAGE;
        }
        IU iu = IU.newBuilder().setUid(iua.getUid()).setRevision(iua.getRevision()).setCategory(iua.getCategory())
                .setOwnerName(iua.getOwnerName()).setCommitted(iua.isCommitted()).setAccessMode(accessMode)
                .setReadOnly(iua.isReadOnly()).setPayloadType("STR").addAllPayload(payloadItems).addAllLinks(links).build();
        String wireFormat = (accessMode == IU.AccessMode.MESSAGE) ? "ipaaca-messageiu" : "ipaaca-iu";
        return new WireContents<ByteBuffer>(ByteBuffer.wrap(iu.toByteArray()), wireFormat);
    }


    @Override
    public UserData<?> deserialize(String wireSchema, ByteBuffer buffer) throws ConversionException
    {
        IU iu;
        try
        {
            iu = IU.newBuilder().mergeFrom(buffer.array()).build();
        }
        catch (InvalidProtocolBufferException e)
        {
            throw new RuntimeException(e);
        }

        if (iu.getAccessMode() == IU.AccessMode.PUSH)
        {
            RemotePushIU iuout = new RemotePushIU(iu.getUid());
            copyIU(iu, iuout);
            return new UserData<RemotePushIU>(iuout, RemotePushIU.class);
        }
        else if(iu.getAccessMode() == IU.AccessMode.MESSAGE)
        {
            RemoteMessageIU iuout = new RemoteMessageIU(iu.getUid());
            copyIU(iu,iuout);
            return new UserData<RemoteMessageIU>(iuout, RemoteMessageIU.class);
        }
        else
        {
            throw new RuntimeException("Trying to deserialize IU with accesmode: "+iu.getAccessMode()+". " +
            		"We can only handle IUs with access mode 'PUSH' or 'MESSAGE' for now!");
        }
    }

    private void copyIU(IU iu, AbstractIU iuout)
    {
        iuout.setCategory(iu.getCategory());
        iuout.committed = iu.getCommitted();
        iuout.setOwnerName(iu.getOwnerName());
        iuout.setRevision(iu.getRevision());
        iuout.setReadOnly(iu.getReadOnly());
        iuout.payload = new Payload(iuout, iu.getPayloadList());
        SetMultimap<String, String> links = HashMultimap.create();
        for (LinkSet ls : iu.getLinksList())
        {
            links.putAll(ls.getType(), ls.getTargetsList());
        }
        iuout.setLinksLocally(links);
    }

}
