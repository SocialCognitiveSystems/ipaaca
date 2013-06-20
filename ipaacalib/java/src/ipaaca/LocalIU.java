package ipaaca;

import ipaaca.protobuf.Ipaaca.IULinkUpdate;
import ipaaca.protobuf.Ipaaca.IUPayloadUpdate;
import ipaaca.protobuf.Ipaaca.LinkSet;
import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;

import com.google.common.collect.SetMultimap;

public class LocalIU extends AbstractIU
{

    private OutputBuffer outputBuffer;

    public OutputBuffer getOutputBuffer()
    {
        return outputBuffer;
    }

    private final Object revisionLock = new Object();

    public Object getRevisionLock()
    {
        return revisionLock;
    }

    public LocalIU()
    {
        super(UUID.randomUUID().toString());
        revision = 1;
        payload = new Payload(this);
    }

    public LocalIU(String category)
    {
        super(UUID.randomUUID().toString());
        this.category = category;
        revision = 1;
        payload = new Payload(this); 
    }
    
    
    // def _set_buffer(self, buffer):
    // if self._buffer is not None:
    // raise Exception('The IU is already in a buffer, cannot move it.')
    // self._buffer = buffer
    // self.owner_name = buffer.unique_name
    // self._payload.owner_name = buffer.unique_name
    //
    protected void setBuffer(OutputBuffer buffer)
    {
        if (outputBuffer != null)
        {
            throw new RuntimeException("The IU is already in a buffer, cannot move it.");
        }
        outputBuffer = buffer;
        ownerName = buffer.getUniqueName();
        // self._payload.owner_name = buffer.unique_name
        super.setBuffer(buffer);
    }

    private void internalCommit(String writerName)
    {

        synchronized (revisionLock)
        {
            if (committed)
            {
                throw new IUCommittedException(this);
            }
            else
            {
                increaseRevisionNumber();
                committed = true;
                if (outputBuffer != null)
                {
                    outputBuffer.sendIUCommission(this, writerName);
                }
            }
        }
    }

    private void increaseRevisionNumber()
    {
        revision++;
    }

    public Payload getPayload()
    {
        return payload;
    }

    // if self.committed:
    // raise IUCommittedError(self)
    // with self.revision_lock:
    // # modify links locally
    // self._increase_revision_number()
    // if self.is_published:
    // # send update to remote holders
    // self.buffer._send_iu_link_update(
    // self,
    // revision=self.revision,
    // is_delta=is_delta,
    // new_links=new_links,
    // links_to_remove=links_to_remove,
    // writer_name=self.owner_name if writer_name is None else writer_name)
    @Override
    void modifyLinks(boolean isDelta, SetMultimap<String, String> linksToAdd, SetMultimap<String, String> linksToRemove, String writerName)
    {
        if (isCommitted())
        {
            throw new IUCommittedException(this);
        }
        synchronized (revisionLock)
        {
            increaseRevisionNumber();
            if (isPublished())
            {
                String wName = null;
                if (getBuffer() != null)
                {
                    wName = getBuffer().getUniqueName();
                }
                if (writerName != null)
                {
                    wName = writerName;
                }
                if (getBuffer() == null)
                {
                    wName = null;
                }
                Set<LinkSet> addSet = new HashSet<LinkSet>();
                for (Entry<String, Collection<String>> entry : linksToAdd.asMap().entrySet())
                {
                    addSet.add(LinkSet.newBuilder().setType(entry.getKey()).addAllTargets(entry.getValue()).build());
                }
                Set<LinkSet> removeSet = new HashSet<LinkSet>();
                for (Entry<String, Collection<String>> entry : linksToRemove.asMap().entrySet())
                {
                    removeSet.add(LinkSet.newBuilder().setType(entry.getKey()).addAllTargets(entry.getValue()).build());
                }
                outputBuffer.sendIULinkUpdate(this,
                        IULinkUpdate.newBuilder().setUid(getUid()).setRevision(getRevision()).setWriterName(wName).setIsDelta(isDelta)
                                .addAllNewLinks(addSet).addAllLinksToRemove(removeSet).build());
            }
        }
    }

    public void setPayload(List<PayloadItem> payloadItems, String writerName)
    {
        synchronized (revisionLock)
        {
            if (committed)
            {
                throw new IUCommittedException(this);
            }
            else
            {
                increaseRevisionNumber();
                String wName = null;
                if (getBuffer() != null)
                {
                    wName = getBuffer().getUniqueName();
                }
                if (writerName != null)
                {
                    wName = writerName;
                }
                if (getBuffer() == null)
                {
                    wName = null;
                }
                payload.set(payloadItems, wName);
            }
        }
    }

    @Override
    void putIntoPayload(String key, String value, String writer)
    {
        synchronized (getRevisionLock())
        {
            // set item locally
            if (isCommitted())
            {
                throw new IUCommittedException(this);
            }
            increaseRevisionNumber();
            if (isPublished())
            {
                // send update to remote holders
                PayloadItem newItem = PayloadItem.newBuilder().setKey(key).setValue(value).setType("") // TODO: fix this, default in .proto?
                        .build();
                IUPayloadUpdate update = IUPayloadUpdate.newBuilder().setUid(getUid()).setRevision(getRevision()).setIsDelta(true)
                        .setWriterName(writer == null ? getOwnerName() : writer).addNewItems(newItem).build();
                getOutputBuffer().sendIUPayloadUpdate(this, update);
            }
        }
    }

    @Override
    public void commit()
    {
        internalCommit(null);
    }

    @Override
    public void commit(String writerName)
    {
        internalCommit(writerName);
    }

    @Override
    void removeFromPayload(Object key, String writer)
    {
        synchronized (getRevisionLock())
        {
            if (isCommitted())
            {
                throw new IUCommittedException(this);
            }
            increaseRevisionNumber();
            if (isPublished())
            {
                // send update to remote holders
                IUPayloadUpdate update = IUPayloadUpdate.newBuilder().setUid(getUid()).setRevision(getRevision()).setIsDelta(true)
                        .setWriterName(writer == null ? getOwnerName() : writer).addKeysToRemove((String) key).build();
                getOutputBuffer().sendIUPayloadUpdate(this, update);
            }
        }

    }

    @Override
    void handlePayloadSetting(List<PayloadItem> newPayload, String writerName)
    {
        if (isPublished())
        {
            IUPayloadUpdate update = IUPayloadUpdate.newBuilder().setUid(getUid()).setRevision(getRevision()).setIsDelta(false)
                    .setWriterName(writerName == null ? getOwnerName() : writerName).addAllNewItems(newPayload).build();
            getOutputBuffer().sendIUPayloadUpdate(this, update);
        }
    }
    
    @Override
    public String toString()
    {
        return "LocalIU with category: "+this.getCategory() + "\nowner: "+getOwnerName()+"\npayload: "+this.getPayload();
    }
}
