package ipaaca.util;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;
import ipaaca.LocalIU;
import ipaaca.OutputBuffer;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.collect.ImmutableSet;

/**
 * A simple key-value blackboard
 * @author hvanwelbergen
 */
public class Blackboard
{
    private final OutputBuffer outBuffer;
    private final InputBuffer inBuffer;
    private final LocalIU iu;
    private final ComponentNotifier notifier;
    private static final String DUMMY_KEY = "DUMMY_KEY";
    public static final String MESSAGE_SUFFIX = "MESSAGE";
    private int dummyValue = 0;
    private List<BlackboardUpdateListener> listeners = Collections.synchronizedList(new ArrayList<BlackboardUpdateListener>());

    public Blackboard(String id, String category)
    {
        this(id, category, "default");
    }

    private void updateListeners()
    {
        synchronized (listeners)
        {
            for (BlackboardUpdateListener listener : listeners)
            {
                listener.update();
            }
        }
    }
    public Blackboard(String id, String category, String channel)
    {
        outBuffer = new OutputBuffer(id, channel);
        iu = new LocalIU(category);
        outBuffer.add(iu);
        outBuffer.registerHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iu, IUEventType type, boolean local)
            {
                updateListeners();
            }
        });
        inBuffer = new InputBuffer(id, ImmutableSet.of(ComponentNotifier.NOTIFY_CATEGORY, category + MESSAGE_SUFFIX), channel);
        notifier = new ComponentNotifier(id, category, ImmutableSet.of(category), new HashSet<String>(), outBuffer, inBuffer);
        notifier.addNotificationHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iuNotify, IUEventType type, boolean local)
            {
                dummyValue++;
                iu.getPayload().put(DUMMY_KEY, "" + dummyValue);
            }
        });
        notifier.initialize();
        inBuffer.registerHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iuMessage, IUEventType type, boolean local)
            {
                iu.getPayload().putAll(iuMessage.getPayload());
                updateListeners();
            }
        }, ImmutableSet.of(category + MESSAGE_SUFFIX));
    }

    public String put(String key, String value)
    {
        return iu.getPayload().put(key, value);
    }

    public void putAll(Map<String, String> newItems)
    {
        iu.getPayload().putAll(newItems);
    }

    /**
     * Get the value corresponding to the key, or null if it is not available
     */
    public String get(String key)
    {
        return iu.getPayload().get(key);
    }

    public void addUpdateListener(BlackboardUpdateListener listener)
    {
        listeners.add(listener);
    }

    public Set<String> keySet()
    {
        return iu.getPayload().keySet();
    }

    public Set<Map.Entry<String, String>> entrySet()
    {
        return iu.getPayload().entrySet();
    }

    public Collection<String> values()
    {
        return iu.getPayload().values();
    }

    public void close()
    {
        outBuffer.close();
        inBuffer.close();
    }
}
