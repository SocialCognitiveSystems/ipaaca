package ipaaca.util;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;
import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;
import ipaaca.protobuf.Ipaaca.IU;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.collect.ImmutableSet;

/**
 * Client to get/set key value pairs on a Blackboard
 * @author hvanwelbergen
 *
 */
public class BlackboardClient
{
    private final InputBuffer inBuffer;
    private final OutputBuffer outBuffer;
    private List<BlackboardUpdateListener> listeners = Collections.synchronizedList(new ArrayList<BlackboardUpdateListener>());
    private final String category;
    
    public BlackboardClient(String id, String category)
    {
        this(id, category, "default");
    }

    public BlackboardClient(String id, String category, String channel)
    {
        this.category = category;
        inBuffer = new InputBuffer(id, ImmutableSet.of(category, ComponentNotifier.NOTIFY_CATEGORY), channel);
        inBuffer.setResendActive(true);
        inBuffer.registerHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iu, IUEventType type, boolean local)
            {
                synchronized (listeners)
                {
                    for (BlackboardUpdateListener listener : listeners)
                    {
                        listener.update();
                    }
                }
            }
        }, ImmutableSet.of(category));
        outBuffer = new OutputBuffer(id);
        ComponentNotifier notifier = new ComponentNotifier(id, category, new HashSet<String>(), ImmutableSet.of(category),
                outBuffer, inBuffer);
        notifier.initialize();
    }

    public void close()
    {
        inBuffer.close();
        outBuffer.close();
    }
    
    public void waitForBlackboardConnection()
    {
        while(inBuffer.getIUs().isEmpty());        
    }
    
    public String get(String key)
    {
        if (inBuffer.getIUs().isEmpty())
        {
            return null;
        }
        return inBuffer.getIUs().iterator().next().getPayload().get(key);
    }

    public void put(String key, String value)
    {
        LocalMessageIU iu = new LocalMessageIU(category+Blackboard.MESSAGE_SUFFIX);
        iu.getPayload().put(key,value);
        outBuffer.add(iu);
    }
    
    public void putAll(Map<String,String> values)
    {
        LocalMessageIU iu = new LocalMessageIU(category+Blackboard.MESSAGE_SUFFIX);
        iu.getPayload().putAll(values);
        outBuffer.add(iu);
    }
    
    private boolean hasIU()
    {
        return !inBuffer.getIUs().isEmpty();
    }
    
    private AbstractIU getIU()
    {
        return inBuffer.getIUs().iterator().next();
    }
    
    public Set<String> keySet()
    {
        if(!hasIU())return new HashSet<>();
        return getIU().getPayload().keySet();
    }

    public Set<Map.Entry<String, String>> entrySet()
    {
        if(!hasIU())return new HashSet<>();
        return getIU().getPayload().entrySet();
    }

    public Collection<String> values()
    {
        if(!hasIU())return new HashSet<>();
        return getIU().getPayload().values();
    }
    
    public void addUpdateListener(BlackboardUpdateListener listener)
    {
        listeners.add(listener);
    }
}
