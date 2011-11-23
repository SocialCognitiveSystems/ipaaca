package ipaaca;

import java.util.List;

import ipaaca.Ipaaca.IUPayloadUpdate;
import ipaaca.Ipaaca.PayloadItem;

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
        this(null);
    }

    public LocalIU(String uid)
    {
        super(uid);
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
    public void setBuffer(OutputBuffer buffer)
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
                outputBuffer.sendIUCommission(this, writerName);
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
        synchronized(getRevisionLock())
        {
            if(isCommitted())
            {
                throw new IUCommittedException(this);
            }
            increaseRevisionNumber();
            if(isPublished())
            {
                //send update to remote holders
                IUPayloadUpdate update = IUPayloadUpdate.newBuilder()
                        .setUid(getUid())
                        .setRevision(getRevision())
                        .setIsDelta(true)
                        .setWriterName(writer==null?getOwnerName():writer)
                        .addKeysToRemove((String)key)
                        .build();                       
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
}
