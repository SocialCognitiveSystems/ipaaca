package ipaaca;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.SetMultimap;

import rsb.RSBException;
import rsb.patterns.RemoteServer;
import ipaaca.Ipaaca;
import ipaaca.Ipaaca.IUCommission;
import ipaaca.Ipaaca.IULinkUpdate;
import ipaaca.Ipaaca.IUPayloadUpdate;
import ipaaca.Ipaaca.LinkSet;
import ipaaca.Ipaaca.PayloadItem;

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

    // def __init__(self, uid, revision, read_only, owner_name, category, type, committed, payload):
    // super(RemotePushIU, self).__init__(uid=uid, access_mode=IUAccessMode.PUSH, read_only=read_only)
    // self._revision = revision
    // self._category = category
    // self.owner_name = owner_name
    // self._type = type
    // self._committed = committed
    // self._payload = RemotePushPayload(remote_push_iu=self, new_payload=payload)
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
    public void commit()
    {
        commit(null);
    }

    void putIntoPayload(String key, String value, String writer)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        if (isReadOnly())
        {
            throw new IUReadOnlyException(this);
        }
        PayloadItem newItem = PayloadItem.newBuilder().setKey(key).setValue(value).setType("").build();// TODO use default type in .proto
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
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }

    // def commit(self):
    // """Commit to this IU."""
    // if self.read_only:
    // raise IUReadOnlyError(self)
    // if self._committed:
    // # ignore commit requests when already committed
    // return
    // else:
    // commission_request = iuProtoBuf_pb2.IUCommission()
    // commission_request.uid = self.uid
    // commission_request.revision = self.revision
    // commission_request.writer_name = self.buffer.unique_name
    // remote_server = self.buffer._get_remote_server(self)
    // new_revision = remote_server.commit(commission_request)
    // if new_revision == 0:
    // raise IUUpdateFailedError(self)
    // else:
    // self._revision = new_revision
    // self._committed = True
    @Override
    public void commit(String writerName)
    {
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

    // def __str__(self):
    // s = "RemotePushIU{ "
    // s += "uid="+self._uid+" "
    // s += "(buffer="+(self.buffer.unique_name if self.buffer is not None else "<None>")+") "
    // s += "owner_name=" + ("<None>" if self.owner_name is None else self.owner_name) + " "
    // s += "payload={ "
    // for k,v in self.payload.items():
    // s += k+":'"+v+"', "
    // s += "} "
    // s += "}"
    // return s
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

    //
    // def _get_payload(self):
    // return self._payload
    public Payload getPayload()
    {
        return payload;
    }

    // def _set_payload(self, new_pl):
    // if self.committed:
    // raise IUCommittedError(self)
    // if self.read_only:
    // raise IUReadOnlyError(self)
    // requested_update = IUPayloadUpdate(
    // uid=self.uid,
    // revision=self.revision,
    // is_delta=False,
    // writer_name=self.buffer.unique_name,
    // new_items=new_pl,
    // keys_to_remove=[])
    // remote_server = self.buffer._get_remote_server(self)
    // new_revision = remote_server.updatePayload(requested_update)
    // if new_revision == 0:
    // raise IUUpdateFailedError(self)
    // else:
    // self._revision = new_revision
    // self._payload = RemotePushPayload(remote_push_iu=self, new_payload=new_pl)
    // payload = property(
    // fget=_get_payload,
    // fset=_set_payload,
    // doc='Payload dictionary of the IU.')
    @Override
    public void setPayload(List<PayloadItem> newItems, String writerName)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
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

    // def _apply_update(self, update):
    // """Apply a IUPayloadUpdate to the IU."""
    // self._revision = update.revision
    // if update.is_delta:
    // for k in update.keys_to_remove: self.payload._remotely_enforced_delitem(k)
    // for k, v in update.new_items.items(): self.payload._remotely_enforced_setitem(k, v)
    // else:
    // # using '_payload' to circumvent the local writing methods
    // self._payload = RemotePushPayload(remote_push_iu=self, new_payload=update.new_items)
    /**
     * Apply a IUPayloadUpdate to the IU.
     * @param update
     */
    public void applyUpdate(IUPayloadUpdate update)
    {
        revision = update.getRevision();
        if (update.getIsDelta())
        {
            for (String key : update.getKeysToRemoveList())
            {
                payload.enforcedRemoveItem(key);
            }
            for (PayloadItem item : update.getNewItemsList())
            {
                payload.enforcedSetItem(item.getKey(), item.getValue());
            }
        }
        else
        {
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

    // def _apply_commission(self):
    // """Apply commission to the IU"""
    // self._committed = True
    public void applyCommmision()
    {
        committed = true;
    }

    @Override
    void removeFromPayload(Object key, String writer)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
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
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }

    // def _modify_payload(self, payload, is_delta=True, new_items={}, keys_to_remove=[], writer_name=None):
    // """Modify the payload: add or remove item from this payload remotely and send update."""
    // if self.committed:
    // raise IUCommittedError(self)
    // if self.read_only:
    // raise IUReadOnlyError(self)
    // requested_update = IUPayloadUpdate(
    // uid=self.uid,
    // revision=self.revision,
    // is_delta=is_delta,
    // writer_name=self.buffer.unique_name,
    // new_items=new_items,
    // keys_to_remove=keys_to_remove)
    // remote_server = self.buffer._get_remote_server(self)
    // new_revision = remote_server.updatePayload(requested_update)
    // if new_revision == 0:
    // raise IUUpdateFailedError(self)
    // else:
    // self._revision = new_revision
    @Override
    void modifyLinks(boolean isDelta, SetMultimap<String, String> linksToAdd, SetMultimap<String, String> linksToRemove, String writerName)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
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
        if (newRevision == 0)
        {
            throw new IUUpdateFailedException(this);
        }
        setRevision(newRevision);
    }

}
