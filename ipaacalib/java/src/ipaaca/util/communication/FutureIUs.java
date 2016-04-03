package ipaaca.util.communication;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

import com.google.common.collect.ImmutableSet;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;

/**
 * Obtain multiple future ius on an specific category. Usage:<br>
 * FutureIUs futures = new FutureIUs(componentx, key);<br>
 * [make componentx send a IU with key=keyvaluedesired1]<br>
 * AbstractIU iu = futures.take(keyvaluedesired1);<br>
 * [make componentx send a IU with key=keyvaluedesired2]
 * AbstractIU iu = futures.take(keyvaluedesired2);<br>
 * ...<br>
 * futures.close();
 * @author hvanwelbergen
 */
public class FutureIUs
{
    private final InputBuffer inBuffer;
    private final Map<String,BlockingQueue<AbstractIU>> resultsMap = Collections.synchronizedMap(new HashMap<>());
    
    public FutureIUs(String category, String idKey)
    {
        inBuffer = new InputBuffer("FutureIUs", ImmutableSet.of(category));
        inBuffer.registerHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iu, IUEventType type, boolean local)
            {
                String id = iu.getPayload().get(idKey);
                resultsMap.putIfAbsent(id, new ArrayBlockingQueue<AbstractIU>(1));
                resultsMap.get(id).offer(iu);
            }
        }, ImmutableSet.of(category));
    }
    
    /**
     * Waits (if necessary) for the IU and take it (can be done only once)     
     */
    public AbstractIU take(String idValue) throws InterruptedException
    {
        resultsMap.putIfAbsent(idValue, new ArrayBlockingQueue<AbstractIU>(1));
        return resultsMap.get(idValue).take();
    }
    
    /**
     * Wait for at most the given time for the IU and take it (can be done only once), return null on timeout        
     */
    public AbstractIU take(String idValue, long timeout, TimeUnit unit) throws InterruptedException
    {
        resultsMap.putIfAbsent(idValue, new ArrayBlockingQueue<AbstractIU>(1));
        return resultsMap.get(idValue).poll(timeout, unit);
    }
    
    public void close()
    {
        inBuffer.close();
    }
}
