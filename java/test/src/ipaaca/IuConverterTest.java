package ipaaca;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import ipaaca.Ipaaca;
import ipaaca.Ipaaca.IU;
import ipaaca.Ipaaca.IU.AccessMode;
import ipaaca.Ipaaca.PayloadItem;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.mockito.Mockito.*;

import org.junit.Before;
import org.junit.Test;

import com.google.protobuf.InvalidProtocolBufferException;

import rsb.converter.ConversionException;
import rsb.converter.ConverterSignature;
import rsb.converter.UserData;
import rsb.converter.WireContents;
import rsb.patterns.RemoteServer;
import static ipaaca.IUTestUtil.*;

public class IuConverterTest
{
    private static final String CATEGORY = "category1";
    private static final String BUFFERNAME = "buffer1";
    private IUConverter converter = new IUConverter(new ConverterSignature("ipaaca-iu", RemotePushIU.class));
    private InputBuffer mockInputBuffer = mock(InputBuffer.class); 
    private RemoteServer mockRemoteServer = mock(RemoteServer.class);

    @Before
    public void setup()
    {
        when(mockInputBuffer.getUniqueName()).thenReturn(BUFFERNAME);
        when(mockInputBuffer.getRemoteServer((AbstractIU)any())).thenReturn(mockRemoteServer);
    }
    
    

    @Test
    public void testSerialize() throws ConversionException, InvalidProtocolBufferException
    {
        RemotePushIU rpIU = new RemotePushIU("iu1");
        rpIU.setRevision(1);
        rpIU.setOwnerName("owner");
        rpIU.setCategory(CATEGORY);
        rpIU.setBuffer(mockInputBuffer);
        rpIU.getPayload().enforcedSetItem("key1", "value1");
        rpIU.getPayload().enforcedSetItem("key2", "value2");
        rpIU.getPayload().enforcedSetItem("key3", "value3");
                
        WireContents<ByteBuffer> wiu = converter.serialize(RemotePushIU.class,rpIU);
        IU iu = IU.newBuilder().mergeFrom(wiu.getSerialization().array()).build();
        assertEqualIU(iu, rpIU);
    }

    public PayloadItem createPayloadItem(String key, String value)
    {
        return PayloadItem.newBuilder()
                .setKey("key1")
                .setValue("value1")
                .setType("")
                .build();        
    }
    
    @Test
    public void testDeSerialize() throws ConversionException
    {
        List<PayloadItem> payload = new ArrayList<PayloadItem>();
        payload.add(createPayloadItem("key1","value1"));
        payload.add(createPayloadItem("key2","value2"));
        payload.add(createPayloadItem("key3","value3"));        
        Ipaaca.IU iu = Ipaaca.IU.newBuilder()
                .setUid("uid1")
                .setRevision(1)
                .setCommitted(false)
                .setOwnerName("owner")
                .setAccessMode(AccessMode.PUSH)
                .setReadOnly(false)
                .setCategory(CATEGORY)                
                .addAllPayload(payload)
                .build();
        
        UserData<?> data = converter.deserialize("", ByteBuffer.wrap(iu.toByteArray()));
        
        assertThat(data.getData(), instanceOf(RemotePushIU.class));
        RemotePushIU rpIU = (RemotePushIU) data.getData();
        assertEqualIU(iu, rpIU);
    }
}
