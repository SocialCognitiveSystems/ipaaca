package ipaaca;

import ipaaca.protobuf.Ipaaca.IUPayloadUpdate;

import java.nio.ByteBuffer;

import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;

import com.google.protobuf.InvalidProtocolBufferException;

public class PayloadConverter implements Converter<ByteBuffer>
{
    private static final String PAYLOAD_WIRESCHEMA = "ipaaca-iu-payload-update";

    @Override
    public UserData<?> deserialize(String wireSchema, ByteBuffer buffer) throws ConversionException
    {
        IUPayloadUpdate pl;
        try
        {
            pl = IUPayloadUpdate.newBuilder().mergeFrom(buffer.array()).build();
        }
        catch (InvalidProtocolBufferException e)
        {
            throw new RuntimeException(e);
        }
        return new UserData<IUPayloadUpdate>(pl, IUPayloadUpdate.class);   
    }

    @Override
    public ConverterSignature getSignature()
    {
        return new ConverterSignature(PAYLOAD_WIRESCHEMA,IUPayloadUpdate.class);
    }

    @Override
    public WireContents<ByteBuffer> serialize(Class<?> typeInfo, Object obj) throws ConversionException
    {
        IUPayloadUpdate pl = (IUPayloadUpdate)obj;
        return new WireContents<ByteBuffer>(ByteBuffer.wrap(pl.toByteArray()),PAYLOAD_WIRESCHEMA);        
    }

}
