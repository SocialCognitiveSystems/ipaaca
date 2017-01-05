package ipaaca.util;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import ipaaca.Initializer;

import org.junit.After;
import org.junit.Test;

import com.google.common.collect.ImmutableMap;

/**
 * Integration tests for the blackboard
 * @author hvanwelbergen
 */
public class BlackboardIntegrationTest
{
    static
    {
        Initializer.initializeIpaacaRsb();
    }
    
    private Blackboard bb = new Blackboard("myblackboard","blackboardx");
    private BlackboardClient bbc;
    
    @After
    public void after()
    {
        bb.close();
        if(bbc!=null)
        {
            bbc.close();
        }
    }
    
    @Test
    public void testGetValueFromBlackboardBeforeConnection()
    {
        bb.put("key1","value1");
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        assertEquals("value1", bbc.get("key1"));        
    }
    
    @Test
    public void testGetValueFromBlackboardAfterConnection() throws InterruptedException
    {
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bb.put("key1","value1");
        Thread.sleep(200);
        assertEquals("value1", bbc.get("key1"));        
    }
    
    @Test
    public void testSetValueOnBlackboard() throws InterruptedException
    {
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bbc.put("key2","value2");
        Thread.sleep(300);
        assertEquals("value2", bb.get("key2"));        
    }
    
    @Test
    public void testBlackboardUpdateHandler()throws InterruptedException
    {
        BlackboardUpdateListener mockListener = mock(BlackboardUpdateListener.class);
        bb.addUpdateListener(mockListener);
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bbc.put("key2","value2");
        Thread.sleep(200);
        bb.put("key2","value3");
        verify(mockListener,times(1)).update();
    }
    
    @Test
    public void testBlackboardClientUpdateHandler() throws InterruptedException
    {
        BlackboardUpdateListener mockListener = mock(BlackboardUpdateListener.class);
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bbc.addUpdateListener(mockListener);
        bb.put("key3","value3");
        Thread.sleep(200);
        bbc.put("key3","value4");
        verify(mockListener,times(1)).update();   // RY: why was this set to 2 before?? 1 update is local
                                                  // changed to one (one is also emitted atm)
    }
    
    @Test
    public void testSetManyValuesOnBlackboard() throws InterruptedException
    {
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        for(int i=0;i<100;i++)
        {
            bbc.put("key"+i,"value"+i);
            bb.put("key"+i,"value"+i);
        }
        Thread.sleep(300);
        assertEquals("value2", bb.get("key2"));        
        assertEquals("value3", bb.get("key3"));
    }
    
    @Test
    public void testSetValuesOnClient() throws InterruptedException
    {
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bbc.putAll(ImmutableMap.of("key1","value1","key2","value2"));
        Thread.sleep(200);
        assertEquals("value1", bb.get("key1"));        
        assertEquals("value2", bb.get("key2"));
    }
    
    @Test
    public void testSetValuesOnBlackBoard() throws InterruptedException
    {
        bbc = new BlackboardClient("myblackboardclient","blackboardx");
        bbc.waitForBlackboardConnection();
        bb.putAll(ImmutableMap.of("key1","value1","key2","value2"));
        Thread.sleep(200);
        assertEquals("value1", bbc.get("key1"));        
        assertEquals("value2", bbc.get("key2"));
    }
}
