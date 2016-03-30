package ipaaca.util.communication;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.concurrent.TimeUnit;

import org.junit.After;
import org.junit.Test;

import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;

/**
 * Unit tests for FutureIUs
 * @author hvanwelbergen
 *
 */
public class FutureIUsTest
{
    private FutureIUs fus = new FutureIUs("cat1","id");
    private final OutputBuffer outBuffer = new OutputBuffer("component1");
    
    @After
    public void cleanup()
    {
        fus.close();
    }
    
    @Test(timeout = 2000)
    public void testSendBeforeTake() throws InterruptedException
    {
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("id", "id1");
        outBuffer.add(message);
        assertEquals(message.getPayload(), fus.take("id1").getPayload());
    }
    
    @Test(timeout = 2000)
    public void testSendAfterTake() throws InterruptedException
    {
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("id", "id1");        
        Runnable send = () -> {
            try
            {
                Thread.sleep(1000);
            }
            catch (Exception e)
            {
                throw new RuntimeException(e);
            }
            outBuffer.add(message);
        };
        new Thread(send).start();
        assertEquals(message.getPayload(), fus.take("id1").getPayload());    
    }
    
    @Test
    public void testNonMatchingKeyValue() throws InterruptedException
    {
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("id", "id2");
        outBuffer.add(message);
        assertNull(fus.take("id1", 1,TimeUnit.SECONDS));
    }
    
    @Test
    public void testMultipleKeyValues() throws InterruptedException
    {
        LocalMessageIU message1 = new LocalMessageIU("cat1");
        message1.getPayload().put("id", "id1");
        LocalMessageIU message2 = new LocalMessageIU("cat1");
        message2.getPayload().put("id", "id2");        
        outBuffer.add(message2);
        outBuffer.add(message1);
        
        assertEquals(message1.getPayload(), fus.take("id1").getPayload());
        assertEquals(message2.getPayload(), fus.take("id2").getPayload());
    }
}
