package ipaaca;

import static org.hamcrest.collection.IsIterableContainingInAnyOrder.containsInAnyOrder;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;

import java.util.EnumSet;
import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.google.common.collect.ImmutableSet;

/**
 * Test communication of the 'MESSAGE' type between IUs
 * @author hvanwelbergen
 * 
 */
public class ComponentMessageCommunicationIntegrationTest
{
    @BeforeClass
    public static void setupStatic()
    {
        Initializer.initializeIpaacaRsb();
    }

    private OutputBuffer outBuffer;
    private InputBuffer inBuffer;
    private LocalMessageIU localIU;
    private CountingEventHandler component1EventHandler;
    private CountingEventHandler component2EventHandler;
    private StoringEventHandler component1StoreHandler = new StoringEventHandler();
    private StoringEventHandler component2StoreHandler = new StoringEventHandler();
    private static final String CATEGORY = "category1";

    @Before
    public void setup()
    {
        outBuffer = new OutputBuffer("component1");

        Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();
        inBuffer = new InputBuffer("component2", categories);
        EnumSet<IUEventType> types = EnumSet.of(IUEventType.ADDED, IUEventType.COMMITTED, IUEventType.UPDATED, IUEventType.MESSAGE);
        component2EventHandler = new CountingEventHandler();
        component1EventHandler = new CountingEventHandler();
        inBuffer.registerHandler(new IUEventHandler(component2EventHandler, types, categories));
        outBuffer.registerHandler(new IUEventHandler(component1EventHandler, types, categories));
        inBuffer.registerHandler(new IUEventHandler(component2StoreHandler, types, categories));
        outBuffer.registerHandler(new IUEventHandler(component1StoreHandler, types, categories));

        localIU = new LocalMessageIU();
        localIU.setCategory(CATEGORY);
        localIU.getPayload().put("key1", "item1");
        localIU.addLinks("INIT", ImmutableSet.of("init1", "init2"));
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
        outBuffer.add(localIU);
        Thread.sleep(200);
        AbstractIU iuIn = inBuffer.getIU(localIU.getUid());
        assertNull(iuIn);
        assertThat(localIU.getLinks("INIT"), containsInAnyOrder("init1", "init2"));
        assertEquals(1, component2EventHandler.getNumberOfMessageEvents(localIU.getUid()));
        assertEquals(0, component1EventHandler.getNumberOfMessageEvents(localIU.getUid()));
        assertEquals(1, component2EventHandler.getNumberOfMessageEvents(localIU.getUid()));
        assertEquals(0, component1EventHandler.getNumberOfMessageEvents(localIU.getUid()));
        assertEquals(localIU.getUid(), component2StoreHandler.getMessageIUs().get(0).getUid());
    }

    @Test
    public void testIUCommit() throws InterruptedException
    {
        outBuffer.add(localIU);
        localIU.commit();
        Thread.sleep(200);
        assertEquals(0, component1EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertEquals(0, component2EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertFalse(component2StoreHandler.getMessageIUs().get(0).isCommitted());
    }

    @Test
    public void testIUCommitBeforePublish() throws InterruptedException
    {
        localIU.commit();
        outBuffer.add(localIU);
        Thread.sleep(200);
        assertEquals(0, component1EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertEquals(0, component2EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertTrue(component2StoreHandler.getMessageIUs().get(0).isCommitted());
    }

    @Test
    public void testIUCommitFromInputBuffer() throws InterruptedException
    {
        outBuffer.add(localIU);
        Thread.sleep(200);
        AbstractIU iuIn = component2StoreHandler.getMessageIUs().get(0);

        iuIn.commit();
        Thread.sleep(200);
        assertFalse(localIU.isCommitted());
        assertEquals(0, component1EventHandler.getNumberOfCommitEvents(localIU.getUid()));
        assertEquals(0, component2EventHandler.getNumberOfCommitEvents(localIU.getUid()));
    }

    @Test
    public void testIUUpdate() throws InterruptedException
    {
        outBuffer.add(localIU);
        Thread.sleep(200);
        AbstractIU iuIn = component2StoreHandler.getMessageIUs().get(0);
        assertNull(iuIn.getPayload().get("key2"));

        localIU.getPayload().put("key2", "value2");
        Thread.sleep(200);
        assertEquals(null, iuIn.getPayload().get("key2"));
        assertEquals(0, component2EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
        assertEquals(0, component1EventHandler.getNumberOfUpdateEvents(localIU.getUid()));
    }

    @Test
    public void testIUUpdateBeforePublish() throws InterruptedException
    {
        localIU.getPayload().put("key2", "value2");
        outBuffer.add(localIU);

        Thread.sleep(200);
        AbstractIU iuIn = component2StoreHandler.getMessageIUs().get(0);
        assertEquals("value2", iuIn.getPayload().get("key2"));
    }

    private void fillBuffer(int val)
    {
        LocalMessageIU iu = new LocalMessageIU();
        StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < 1000000; i++)
        {
            buffer.append(val);
        }
        iu.setCategory("");
        iu.getPayload().put("x", buffer.toString());
        outBuffer.add(iu);
    }

    @Test
    public void testManyMessages()
    {
        for (int i = 0; i < 1000; i++)
        {
            fillBuffer(i);
        }
    }
}
