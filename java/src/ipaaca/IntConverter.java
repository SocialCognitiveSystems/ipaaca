package ipaaca;

import ipaaca.protobuf.Ipaaca.IntMessage;

import java.nio.ByteBuffer;
import rsb.converter.ConversionException;
import rsb.converter.Converter;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;

import com.google.protobuf.InvalidProtocolBufferException;

/**
 * Serializer/deserializer for ints
 * @author hvanwelbergen
 * 
 */
public class IntConverter implements Converter<ByteBuffer>
{

    @Override
    public ConverterSignature getSignature()
    {
        return new ConverterSignature("int32", Integer.class);
    }

    @Override
    public WireContents<ByteBuffer> serialize(Class<?> typeInfo, Object obj) throws ConversionException
    {
        Integer intVal = (Integer) obj;
        IntMessage message = IntMessage.newBuilder().setValue(intVal).build();

        return new WireContents<ByteBuffer>(ByteBuffer.wrap(message.toByteArray()), "int32");
    }

    @Override
    public UserData<?> deserialize(String wireSchema, ByteBuffer buffer) throws ConversionException
    {
        IntMessage message;
        try
        {
            message = IntMessage.newBuilder().mergeFrom(buffer.array()).build();
        }
        catch (InvalidProtocolBufferException e)
        {
            throw new RuntimeException(e);
        }

        return new UserData<Integer>(message.getValue(), Integer.class);
    }
}
