package ipaaca;

import ipaaca.Ipaaca.IULinkUpdate;

import java.nio.ByteBuffer;

import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;

import com.google.protobuf.InvalidProtocolBufferException;

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
        return new ConverterSignature(LINKUPDATE_WIRESCHEMA,IULinkUpdate.class);
    }

    @Override
    public WireContents<ByteBuffer> serialize(Class<?> typeInfo, Object obj) throws ConversionException
    {
        IULinkUpdate pl = (IULinkUpdate)obj;
        return new WireContents<ByteBuffer>(ByteBuffer.wrap(pl.toByteArray()),LINKUPDATE_WIRESCHEMA);        
    }

}
