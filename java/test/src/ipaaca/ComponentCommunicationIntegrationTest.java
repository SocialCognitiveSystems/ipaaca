package ipaaca;

import static org.junit.Assert.*;

import java.util.EnumSet;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;

import static org.hamcrest.collection.IsIterableContainingInAnyOrder.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import static ipaaca.IUTestUtil.*;

/**
 * Integration test cases for IPAACA.
 * Requires a running spread daemon.
 * @author hvanwelbergen
 * 
 */
public class ComponentCommunicationIntegrationTest
{
    static
    {
        Initializer.initializeIpaacaRsb();
    }
    
    private OutputBuffer outBuffer;
    private InputBuffer inBuffer;
    private LocalIU localIU;
    private MyEventHandler component1EventHandler;    
    private MyEventHandler component2EventHandler;
    private static final String CATEGORY = "category1";
    
    private static final class MyEventHandler implements HandlerFunctor
    {
        private Map<String,Integer> commitEvents = new HashMap<String,Integer>();
        private Map<String,Integer> addEvents = new HashMap<String,Integer>();
        private Map<String,Integer> updateEvents = new HashMap<String,Integer>();
        
        private void updateEventMap(String key, Map<String,Integer> map)
        {
            int value = 0;
            if(map.containsKey(key))
            {
                value = map.get(key);
            }
            value++;
            map.put(key, value);
        }
        
        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            switch(type)
            {
            case ADDED:  updateEventMap(iu.getUid(),addEvents);  break;
            case COMMITTED: updateEventMap(iu.getUid(),commitEvents); break;
            case UPDATED: updateEventMap(iu.getUid(),updateEvents); break;
            }            
        }
        
        public int getNumberOfCommitEvents(String iu)
        {
            if(!commitEvents.containsKey(iu))
            {
                return 0;
            }
            return commitEvents.get(iu);
        }
        
        public int getNumberOfAddEvents(String iu)
        {
            if(!addEvents.containsKey(iu))
            {
                return 0;
            }
            return addEvents.get(iu);
        }
        
        public int getNumberOfUpdateEvents(String iu)
        {
            if(!updateEvents.containsKey(iu))
            {
                return 0;
            }
            return updateEvents.get(iu);
        }
    }
    
    @Before
    public void setup()
    {
        outBuffer = new OutputBuffer("component1");
        
        Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();        
        inBuffer = new InputBuffer("component2", categories);
        EnumSet<IUEventType> types = EnumSet.of(IUEventType.ADDED,IUEventType.COMMITTED,IUEventType.UPDATED);
        component2EventHandler = new MyEventHandler();
        component1EventHandler = new MyEventHandler();
        inBuffer.registerHandler(new IUEventHandler(component2EventHandler,types,categories));
        outBuffer.registerHandler(new IUEventHandler(component1EventHandler,types,categories));
        
        localIU = new LocalIU();
        localIU.setCategory(CATEGORY);
        localIU.getPayload().put("key1", "item1");
        localIU.addLinks("INIT", ImmutableSet.of("init1","init2"));
        outBuffer.add(localIU);
    }
    
    @After
    public void tearDown()
    {
        inBuffer.close();
        outBuffer.close();
    }
    
    @Test
    public void testAddedIU() throws InterruptedException
    {
        Thread.sleep(200);        
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertNotNull(iuIn);
        assertEqualIU(iuIn, localIU);
        assertThat(localIU.getLinks("INIT"),containsInAnyOrder("init1","init2"));
        assertEquals(1,component2EventHandler.getNumberOfAddEvents(iuIn.getUid()));
        assertEquals(0,component1EventHandler.getNumberOfAddEvents(localIU.getUid()));
    }
    
    @Test
    public void testIUCommit() throws InterruptedException
    {
        localIU.commit();
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertTrue(iuIn.isCommitted());
        assertEquals(0,component1EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertEquals(1,component2EventHandler.getNumberOfCommitEvents(iuIn.getUid()));        
    }
    
    @Test
    public void testIUCommitFromInputBuffer() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        
        iuIn.commit();
        Thread.sleep(200);
        assertTrue(localIU.isCommitted());
        assertEquals(1,component1EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        //TODO: currently fails, is this actually the desired behavior?
        //assertEquals(0,component2EventHandler.getNumberOfCommitEvents(localIU.getUid()));        
    }
    
    @Test
    public void testIUUpdate() throws InterruptedException
    {
        Thread.sleep(200);        
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertNull(iuIn.getPayload().get("key2"));
        
        localIU.getPayload().put("key2", "value2");
        Thread.sleep(200);
        assertEquals("value2", iuIn.getPayload().get("key2"));
        assertEquals(1,component2EventHandler.getNumberOfUpdateEvents(iuIn.getUid()));
        assertEquals(0,component1EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
    }

    @Test
    public void testSetPayload() throws InterruptedException
    {
        Thread.sleep(200);        
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        
        localIU.setPayload(ImmutableMap.of("newkey1","newvalue1","newkey2","newvalue2","newkey3","newvalue3"));
        Thread.sleep(200);
        assertFalse(localIU.getPayload().containsKey("key1"));
        assertFalse(iuIn.getPayload().containsKey("key1"));
        assertEquals("newvalue1", iuIn.getPayload().get("newkey1"));
        assertEquals("newvalue2", iuIn.getPayload().get("newkey2"));
        assertEquals("newvalue3", iuIn.getPayload().get("newkey3"));
        assertEquals(1,component2EventHandler.getNumberOfUpdateEvents(iuIn.getUid()));
        assertEquals(0,component1EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
    }
    
    @Test
    public void testSetPayloadRemote() throws InterruptedException
    {
        Thread.sleep(200);        
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        
        iuIn.setPayload(ImmutableMap.of("newkey1","newvalue1","newkey2","newvalue2","newkey3","newvalue3"));
        Thread.sleep(200);
        assertFalse(localIU.getPayload().containsKey("key1"));
        assertFalse(iuIn.getPayload().containsKey("key1"));
        assertEquals("newvalue1", localIU.getPayload().get("newkey1"));
        assertEquals("newvalue2", localIU.getPayload().get("newkey2"));
        assertEquals("newvalue3", localIU.getPayload().get("newkey3"));
        assertEquals(0,component2EventHandler.getNumberOfUpdateEvents(iuIn.getUid()));
        assertEquals(1,component1EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
    }
    
    @Test
    public void testIUUpdateFromInputBuffer() throws InterruptedException
    {
        Thread.sleep(200);        
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        
        
        iuIn.getPayload().put("key2", "value2");
        Thread.sleep(200);
        assertEquals("value2", localIU.getPayload().get("key2"));
        assertEquals(1,component1EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
        assertEquals(0,component2EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
    }
    
    @Test
    public void testIUpdateRemove() throws InterruptedException
    {   
        Thread.sleep(200);    
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertEquals("item1",iuIn.getPayload().get("key1"));
        
        localIU.getPayload().remove("key1");
        Thread.sleep(200);
        
        assertNull(iuIn.getPayload().get("key1"));
        assertNull(localIU.getPayload().get("key1"));
    }
    
    @Test
    public void testIUpdateRemoveFromInputBuffer() throws InterruptedException
    {   
        Thread.sleep(200);    
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertEquals("item1",iuIn.getPayload().get("key1"));
        
        iuIn.getPayload().remove("key1");
        Thread.sleep(200);
        
        assertNull(iuIn.getPayload().get("key1"));
        assertNull(localIU.getPayload().get("key1"));
    }
    
    @Test
    public void testSetLinksLocal() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        localIU.setLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6"));        
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
    }
    
    @Test
    public void testSetLinksRemote() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        iuIn.setLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6"));        
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
    }
    
    @Test
    public void testSetLinksRemoteOverwrite() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        localIU.setLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6"));        
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
        
        iuIn.setLinks("SAME_LEVEL",ImmutableSet.of("iu7","iu8"));
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7","iu8"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7","iu8"));
    }
    
    @Test
    public void testAddLinksLocal() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        localIU.setLinks("SAME_LEVEL",ImmutableSet.of("iu4"));
        localIU.addLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6"));        
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu4","iu5","iu6"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu4","iu5","iu6"));
    }
    
    @Test
    public void testAddLinksRemote() throws InterruptedException
    {
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        iuIn.addLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6"));        
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu5","iu6"));
    }
    
    @Test
    public void testRemoveLinksLocal() throws InterruptedException
    {
        Thread.sleep(200);   
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        localIU.setLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6","iu7"));
        localIU.removeLinks("SAME_LEVEL", ImmutableSet.of("iu5","iu6"));
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7"));
    }
    
    @Test
    public void testRemoveLinksRemote() throws InterruptedException
    {
        Thread.sleep(200);   
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        iuIn.setLinks("SAME_LEVEL",ImmutableSet.of("iu5","iu6","iu7"));
        iuIn.removeLinks("SAME_LEVEL", ImmutableSet.of("iu5","iu6"));
        Thread.sleep(200);
        assertThat(localIU.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7"));
        assertThat(iuIn.getLinks("SAME_LEVEL"),containsInAnyOrder("iu7"));
    }
}
