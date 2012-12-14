package ipaaca;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.doAnswer;
import ipaaca.util.ComponentNotifier;

import java.util.Set;

import org.hamcrest.collection.IsIterableContainingInAnyOrder;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import com.google.common.base.Joiner;
import com.google.common.collect.ImmutableSet;

/**
 * Unit tests for the ComponentNotifier
 * @author hvanwelbergen
 */
public class ComponentNotifierTest
{
    private static final ImmutableSet<String> RECV_CAT = ImmutableSet.of("testrec1", "testrc2");
    private static final ImmutableSet<String> SEND_CAT = ImmutableSet.of("testsnd1", "testsnd2", "testsnd3");
    private OutputBuffer mockOutBuffer = mock(OutputBuffer.class);
    private InputBuffer mockInBuffer = mock(InputBuffer.class);
    private IUEventHandler inputHandler;

    private ComponentNotifier notifier = new ComponentNotifier("testcomp", "testfunc", SEND_CAT, RECV_CAT, mockOutBuffer, mockInBuffer);

    @Before
    public void setup()
    {
        doAnswer(new Answer<Void>()
        {
            @Override
            public Void answer(InvocationOnMock invocation) throws Throwable
            {
                IUEventHandler handler = (IUEventHandler) (invocation.getArguments()[0]);
                inputHandler = handler;
                return null;
            }
        }).when(mockInBuffer).registerHandler(any(IUEventHandler.class));
        notifier.initialize();
    }

    @Test
    public void testNotifyAtInit()
    {
        ArgumentCaptor<LocalIU> argument = ArgumentCaptor.forClass(LocalIU.class);
        verify(mockOutBuffer).add(argument.capture());
        LocalIU iu = argument.getValue();
        assertEquals(ComponentNotifier.NOTIFY_CATEGORY, iu.getCategory());
        assertEquals("new", iu.getPayload().get(ComponentNotifier.STATE));
        assertThat(ImmutableSet.copyOf(iu.getPayload().get(ComponentNotifier.RECEIVE_CATEGORIES).split(",")),
                IsIterableContainingInAnyOrder.containsInAnyOrder(RECV_CAT.toArray(new String[0])));
        assertThat(ImmutableSet.copyOf(iu.getPayload().get(ComponentNotifier.SEND_CATEGORIES).split(",")),
                IsIterableContainingInAnyOrder.containsInAnyOrder(SEND_CAT.toArray(new String[0])));
    }

    private void sendNotify(String state, Set<String> receiveCats)
    {

        AbstractIU mockIUNotify = mock(AbstractIU.class);
        Payload mockNotifyPayload = mock(Payload.class);
        when(mockIUNotify.getCategory()).thenReturn(ComponentNotifier.NOTIFY_CATEGORY);
        when(mockIUNotify.getPayload()).thenReturn(mockNotifyPayload);
        when(mockInBuffer.getIU("iuNotify")).thenReturn(mockIUNotify);
        when(mockNotifyPayload.get(ComponentNotifier.STATE)).thenReturn(state);
        when(mockNotifyPayload.get(ComponentNotifier.NAME)).thenReturn("namex");
        when(mockNotifyPayload.get(ComponentNotifier.SEND_CATEGORIES)).thenReturn("");
        when(mockNotifyPayload.get(ComponentNotifier.RECEIVE_CATEGORIES)).thenReturn(Joiner.on(",").join(receiveCats));
        inputHandler.call(mockInBuffer, "iuNotify", false, IUEventType.ADDED, ComponentNotifier.NOTIFY_CATEGORY);
    }

    @Test
    public void testNotifyAtNotifyNew() throws Exception
    {
        sendNotify("new", ImmutableSet.of("testsnd1"));
        ArgumentCaptor<LocalIU> argument = ArgumentCaptor.forClass(LocalIU.class);
        verify(mockOutBuffer, times(2)).add(argument.capture());
        LocalIU iu = argument.getAllValues().get(1);
        assertEquals("componentNotify", iu.getCategory());
        assertEquals("old", iu.getPayload().get("state"));
    }
    
    @Test
    public void testNoNotifyAtNotifyOld() throws Exception
    {
        sendNotify("old", ImmutableSet.of("testsnd1"));        
        ArgumentCaptor<LocalIU> argument = ArgumentCaptor.forClass(LocalIU.class);
        verify(mockOutBuffer,times(1)).add(argument.capture());        
    }
}
