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

import ipaaca.protobuf.Ipaaca;
import ipaaca.protobuf.Ipaaca.IUCommission;
import ipaaca.protobuf.Ipaaca.IULinkUpdate;
import ipaaca.protobuf.Ipaaca.IUPayloadUpdate;
import ipaaca.protobuf.Ipaaca.LinkSet;
import ipaaca.protobuf.Ipaaca.PayloadItem;
import ipaaca.protobuf.Ipaaca.IUPayloadUpdate.Builder;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import rsb.RSBException;
import rsb.patterns.RemoteServer;

import com.google.common.collect.SetMultimap;

/**
 * A remote IU with access mode 'PUSH'.
 * @author hvanwelbergen
 */
public class RemotePushIU extends AbstractIU
{
    private final static Logger logger = LoggerFactory.getLogger(RemotePushIU.class.getName());
    private InputBuffer inputBuffer;

    public InputBuffer getInputBuffer()
    {
        return inputBuffer;
    }

    public RemotePushIU(String uid)
    {
        super(uid);
        payload = new Payload(this);
    }

    public void setBuffer(InputBuffer buffer)
    {
        inputBuffer = buffer;
        super.setBuffer(buffer);
    }

    @Override
    public void retract()
    {
        logger.info("Retracting a RemoteIU has no effect.");
    }

    void putIntoPayload(String key, String value, String writer)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        PayloadItem newItem = PayloadItem.newBuilder().setKey(key).setValue(value).setType("STR").build();
        IUPayloadUpdate update = IUPayloadUpdate.newBuilder().setIsDelta(true).setUid(getUid()).setRevision(getRevision())
                .setWriterName(getBuffer().getUniqueName()).addNewItems(newItem).build();

        RemoteServer server = getInputBuffer().getRemoteServer(this);
        logger.debug("Remote server has methods {}", server.getMethods());
        int newRevision;
        try
        {
            newRevision = (Integer) server.call("updatePayload", update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        catch (ExecutionException e)
        {
            throw new RuntimeException(e);
        }
        catch (TimeoutException e)
        {
            throw new RuntimeException(e);
        }
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }

    @Override
    void putIntoPayload(Map<? extends String, ? extends String> newItems, String writer)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        Builder builder = IUPayloadUpdate.newBuilder().setUid(getUid()).setRevision(getRevision()).setIsDelta(true)
                .setWriterName(getBuffer().getUniqueName());
        for (Map.Entry<? extends String, ? extends String> item : newItems.entrySet())
        {
            PayloadItem newItem = PayloadItem.newBuilder().setKey(item.getKey()).setValue(item.getValue()).setType("STR")
                    .build();
            builder.addNewItems(newItem);

        }
        IUPayloadUpdate update = builder.build();

        RemoteServer server = getInputBuffer().getRemoteServer(this);
        logger.debug("Remote server has methods {}", server.getMethods());
        int newRevision;
        try
        {
            newRevision = (Integer) server.call("updatePayload", update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        catch (ExecutionException e)
        {
            throw new RuntimeException(e);
        }
        catch (TimeoutException e)
        {
            throw new RuntimeException(e);
        }
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        System.err.print("************************ ");
        System.err.println(newRevision);
        setRevision(newRevision);
    }

    @Override
    public void commit()
    {
        commit(null);
    }

    @Override
    public void commit(String writerName)
    {
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        if (isCommitted())
        {
            return;
        }
        else
        {
            IUCommission iuc = Ipaaca.IUCommission.newBuilder().setUid(getUid()).setRevision(getRevision())
                    .setWriterName(getBuffer().getUniqueName()).build();
            RemoteServer server = inputBuffer.getRemoteServer(this);
            int newRevision;
            try
            {
                newRevision = (Integer) server.call("commit", iuc);
            }
            catch (RSBException e)
            {
                throw new RuntimeException(e);
            }
            catch (ExecutionException e)
            {
                throw new RuntimeException(e);
            }
            catch (TimeoutException e)
            {
                throw new RuntimeException(e);
            }
            if (newRevision == 0)
            {
                throw new IUCommittedException(this);
            }
            else
            {
                revision = newRevision;
                committed = true;
            }
        }
    }

    @Override
    public String toString()
    {
        StringBuffer b = new StringBuffer();
        b.append("RemotePushIU{ ");
        b.append("uid=" + getUid() + " ");
        b.append("(buffer=" + (getBuffer() != null ? getBuffer().getUniqueName() : "<None>") + ") ");
        b.append("owner_name=" + (getOwnerName() == null ? "<None>" : getOwnerName()) + " ");
        b.append("payload={ ");
        for (Entry<String, String> entry : getPayload().entrySet())
        {
            b.append(entry.getKey() + ":'" + entry.getValue() + "', ");
        }
        b.append("} ");
        b.append("} ");
        return b.toString();
    }

    public Payload getPayload()
    {
        return payload;
    }

    @Override
    public void setPayload(List<PayloadItem> newItems, String writerName)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }

        IUPayloadUpdate iuu = IUPayloadUpdate.newBuilder().setRevision(getRevision()).setIsDelta(false).setUid(getUid())
                .addAllNewItems(newItems).setWriterName(getBuffer() != null ? getBuffer().getUniqueName() : "").build();
        RemoteServer server = inputBuffer.getRemoteServer(this);
        int newRevision;
        try
        {
            newRevision = (Integer) server.call("updatePayload", iuu);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        catch (ExecutionException e)
        {
            throw new RuntimeException(e);
        }
        catch (TimeoutException e)
        {
            throw new RuntimeException(e);
        }
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        else
        {
            revision = newRevision;
            payload = new Payload(this, newItems);
        }
    }

    /**
     * Apply a IUPayloadUpdate to the IU.
     * @param update
     */
    public void applyUpdate(IUPayloadUpdate update) {
    	revision = update.getRevision();
    	if (update.getIsDelta()) {
    		for (String key : update.getKeysToRemoveList()) {
    			payload.enforcedRemoveItem(key);
    		}
    		for (PayloadItem item : update.getNewItemsList()) {
    			if (item.getType().equals("STR")) {
    				payload.enforcedSetItem(item.getKey(), item.getValue());
    			} else if (item.getType().equals("JSON")) {
    				String value = item.getValue();
    				if (value.startsWith("\"")) {
    					payload.enforcedSetItem(item.getKey(), value.replaceAll("\\\"", ""));
    				} else if (value.startsWith("{") || value.startsWith("[") || value.matches("true") || value.matches("false") || value.matches("-?[0-9]*[.,]?[0-9][0-9]*.*")) { 
    					payload.enforcedSetItem(item.getKey(), value);
    				} else if (value.equals("null")) {
    					payload.enforcedSetItem(item.getKey(), "");
    				}
    			}
    		}
    	} else {
    		payload = new Payload(this, update.getNewItemsList());
    	}
    }

    public void applyLinkUpdate(IULinkUpdate update)
    {
        revision = update.getRevision();
        if (update.getIsDelta())
        {
            for (LinkSet ls : update.getLinksToRemoveList())
            {
                for (String removeValue : ls.getTargetsList())
                {
                    links.remove(ls.getType(), removeValue);
                }
            }
            for (LinkSet ls : update.getNewLinksList())
            {
                links.putAll(ls.getType(), ls.getTargetsList());
            }
        }
        else
        {
            links.clear();
            for (LinkSet ls : update.getNewLinksList())
            {
                links.putAll(ls.getType(), ls.getTargetsList());
            }
        }
    }

    @Override
    void handlePayloadSetting(List<PayloadItem> newPayload, String writerName)
    {

    }

    public void applyCommmision()
    {
        committed = true;
    }

    public void applyRetraction()
    {
        retracted = true;
    }

    @Override
    void removeFromPayload(Object key, String writer)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        IUPayloadUpdate update = IUPayloadUpdate.newBuilder().setIsDelta(true).setUid(getUid()).setRevision(getRevision())
                .setWriterName(getBuffer().getUniqueName()).addKeysToRemove((String) key).build();
        RemoteServer server = getInputBuffer().getRemoteServer(this);
        int newRevision;
        try
        {
            newRevision = (Integer) server.call("updatePayload", update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        catch (ExecutionException e)
        {
            throw new RuntimeException(e);
        }
        catch (TimeoutException e)
        {
            throw new RuntimeException(e);
        }
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }


    @Override
    void modifyLinks(boolean isDelta, SetMultimap<String, String> linksToAdd, SetMultimap<String, String> linksToRemove, String writerName)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isRetracted())
        {
            throw new IURetractedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        RemoteServer server = inputBuffer.getRemoteServer(this);
        Set<LinkSet> removeLinkSet = new HashSet<LinkSet>();
        for (Entry<String, Collection<String>> entry : linksToRemove.asMap().entrySet())
        {
            removeLinkSet.add(LinkSet.newBuilder().setType(entry.getKey()).addAllTargets(entry.getValue()).build());
        }
        Set<LinkSet> newLinkSet = new HashSet<LinkSet>();
        for (Entry<String, Collection<String>> entry : linksToAdd.asMap().entrySet())
        {
            newLinkSet.add(LinkSet.newBuilder().setType(entry.getKey()).addAllTargets(entry.getValue()).build());
        }

        IULinkUpdate update = IULinkUpdate.newBuilder().addAllLinksToRemove(removeLinkSet).addAllNewLinks(newLinkSet).setIsDelta(isDelta)
                .setWriterName(getBuffer() != null ? getBuffer().getUniqueName() : "").setUid(getUid()).setRevision(getRevision()).build();
        int newRevision;
        try
        {
            newRevision = (Integer) server.call("updateLinks", update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        catch (ExecutionException e)
        {
            throw new RuntimeException(e);
        }
        catch (TimeoutException e)
        {
            throw new RuntimeException(e);
        }
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }

}
