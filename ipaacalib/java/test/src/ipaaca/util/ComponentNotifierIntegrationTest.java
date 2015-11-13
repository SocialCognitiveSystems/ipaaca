package ipaaca.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventType;
import ipaaca.Initializer;
import ipaaca.InputBuffer;
import ipaaca.LocalIU;
import ipaaca.OutputBuffer;

import java.util.Set;

import lombok.Getter;

import org.junit.After;
import org.junit.Test;

import com.google.common.collect.ImmutableSet;

/**
 * Integration test for the ComponentNotifier, connects two of them. Requires a running spread daemon.
 * @author hvanwelbergen
 * 
 */
public class ComponentNotifierIntegrationTest
{
    private ComponentNotifier notifier1;
    private ComponentNotifier notifier2;
    private InputBuffer inBuffer;
    private OutputBuffer outBuffer;
    private static final String OTHER_CATEGORY="OTHER";
    static
    {
        Initializer.initializeIpaacaRsb();
    }

    private class MyHandlerFunctor implements HandlerFunctor
    {
        @Getter
        private volatile int numCalled = 0;

        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            numCalled++;
        }
    }

    @After
    public void after()
    {
        if (inBuffer != null)
        {
            inBuffer.close();
        }
        if (outBuffer != null)
        {
            outBuffer.close();
        }
    }

    private ComponentNotifier setupCompNotifier(String id, Set<String> sendList, Set<String> recvList)
    {
        inBuffer = new InputBuffer(id + "in", ImmutableSet.of(ComponentNotifier.NOTIFY_CATEGORY));
        outBuffer = new OutputBuffer(id + "out");
        return new ComponentNotifier(id, "test", ImmutableSet.copyOf(sendList), ImmutableSet.copyOf(recvList), outBuffer, inBuffer);
    }

    private ComponentNotifier setupCompNotifierWithOtherCategoryInputBuffer(String id, Set<String> sendList, Set<String> recvList)
    {
        inBuffer = new InputBuffer(id + "in", ImmutableSet.of(ComponentNotifier.NOTIFY_CATEGORY, OTHER_CATEGORY));
        outBuffer = new OutputBuffer(id + "out");
        return new ComponentNotifier(id, "test", ImmutableSet.copyOf(sendList), ImmutableSet.copyOf(recvList), outBuffer, inBuffer);
    }
    
    @Test
    public void testSelf() throws InterruptedException
    {
        notifier1 = setupCompNotifier("not1", ImmutableSet.of("a1", "b1"), ImmutableSet.of("a3", "b1"));
        MyHandlerFunctor h1 = new MyHandlerFunctor();
        notifier1.addNotificationHandler(h1);

        notifier1.initialize();
        Thread.sleep(500);
        assertEquals(0, h1.getNumCalled());
    }

    @Test
    public void testTwo() throws InterruptedException
    {
        notifier1 = setupCompNotifier("not1", ImmutableSet.of("a1", "b1"), ImmutableSet.of("a3", "b2"));
        notifier2 = setupCompNotifier("not2", ImmutableSet.of("a2", "b2"), ImmutableSet.of("a3", "b1"));
        MyHandlerFunctor h1 = new MyHandlerFunctor();
        MyHandlerFunctor h2 = new MyHandlerFunctor();
        notifier1.addNotificationHandler(h1);
        notifier2.addNotificationHandler(h2);

        notifier1.initialize();
        Thread.sleep(500);
        notifier2.initialize();
        Thread.sleep(500);

        assertEquals(1, h1.getNumCalled());
        assertEquals(1, h2.getNumCalled());
    }
    
    @Test
    public void testOtherCategoryInInputBuffer() throws InterruptedException
    {
        notifier1 = setupCompNotifierWithOtherCategoryInputBuffer("not1", ImmutableSet.of("a1", "b1"), ImmutableSet.of("a3", "b1"));
        MyHandlerFunctor h1 = new MyHandlerFunctor();
        notifier1.addNotificationHandler(h1);
        
        OutputBuffer out = new OutputBuffer("out");
        LocalIU iu = new LocalIU(OTHER_CATEGORY);
        out.add(iu);
        Thread.sleep(500);
        assertEquals(0, h1.getNumCalled());
        assertNotNull(inBuffer.getIU(iu.getUid()));
    }
}
