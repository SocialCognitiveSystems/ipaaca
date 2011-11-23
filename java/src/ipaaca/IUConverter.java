package ipaaca;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map.Entry;

import com.google.protobuf.InvalidProtocolBufferException;

import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;
import ipaaca.Ipaaca.IU;
import ipaaca.Ipaaca.PayloadItem;

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
        AbstractIU iua = (AbstractIU)obj;
        List<PayloadItem> payloadItems = new ArrayList<PayloadItem>();
        for(Entry<String, String> entry:iua.getPayload().entrySet())
        {
            payloadItems.add(PayloadItem.newBuilder()
                    .setKey(entry.getKey())
                    .setValue(entry.getValue())
                    .setType("")
                    .build());
        }
        
        IU iu = IU.newBuilder()
                .setUid(iua.getUid())
                .setRevision(iua.getRevision())
                .setCategory(iua.getCategory())
                .setType("")
                .setOwnerName(iua.getOwnerName())
                .setCommitted(iua.isCommitted())
                .setAccessMode(IU.AccessMode.PUSH) //TODO for other access modes (also in Python version)
                .setReadOnly(iua.isReadOnly())
                .addAllPayload(payloadItems)
                .build();
        return new WireContents<ByteBuffer>(ByteBuffer.wrap(iu.toByteArray()),"ipaaca-remotepushiu");        
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

        if(iu.getAccessMode() == IU.AccessMode.PUSH)
        {
            RemotePushIU iuout = new RemotePushIU(iu.getUid());
            iuout.setCategory(iu.getCategory());
            iuout.committed = iu.getCommitted();
            iuout.setOwnerName(iu.getOwnerName());
            iuout.setRevision(iu.getRevision());
            iuout.setReadOnly(iu.getReadOnly());            
            iuout.payload = new Payload(iuout,iu.getPayloadList());       
            return new UserData<RemotePushIU>(iuout, RemotePushIU.class);            
        }
        else
        {
            throw new RuntimeException("We can only handle IUs with access mode 'PUSH' for now!");
        }
    }

}
