package ipaaca.util.communication;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;

/**
 * Unit tests for the FutureIU
 * @author hvanwelbergen
 *
 */
public class FutureIUTest
{
    private final OutputBuffer outBuffer = new OutputBuffer("component1");

    @Test(timeout = 2000)
    public void testSendBeforeTake() throws InterruptedException
    {
        FutureIU fu = new FutureIU("cat1", "status", "started");
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("status", "started");
        outBuffer.add(message);
        assertEquals(message.getPayload(), fu.take().getPayload());
    }

    @Test(timeout = 2000)
    public void testSendAfterTake() throws InterruptedException
    {
        FutureIU fu = new FutureIU("cat1", "status", "started");
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("status", "started");
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
        assertEquals(message.getPayload(), fu.take().getPayload());
    }
    
    @Test
    public void testInvalidKeyValue() throws InterruptedException
    {
        FutureIU fu = new FutureIU("cat1", "status", "started");
        LocalMessageIU message = new LocalMessageIU("cat1");
        message.getPayload().put("status", "cancelled");
        assertNull(fu.take(1,TimeUnit.SECONDS));
    }
}
