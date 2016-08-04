package ipaaca.util.communication;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

import com.google.common.collect.ImmutableSet;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventHandler;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;

/**
 * Obtain a IU in the future. Usage:<br>
 * FutureIU fu = FutureIU("componentx", "status", "started"); //wait for componentx to send a message that is it fully started<br>
 * [Start componentx, assumes that component x will send a message or other iu with status=started in the payload]<br>
 * AbstractIU iu = fu.take(); //get the actual IU
 * @author hvanwelbergen
 */
public class FutureIU
{
    private final InputBuffer inBuffer;
    private final BlockingQueue<AbstractIU> queue = new ArrayBlockingQueue<AbstractIU>(1);
    private final IUEventHandler handler;

    public FutureIU(String category, final String idKey, final String idVal, InputBuffer inBuffer)
    {
        this.inBuffer = inBuffer;
        handler = new IUEventHandler(new HandlerFunctor()
        {
            @Override
            public void handle(AbstractIU iu, IUEventType type, boolean local)
            {
                String id = iu.getPayload().get(idKey);
                if (idVal.equals(id))
                {
                    queue.offer(iu);
                }
            }
        }, ImmutableSet.of(category));
        inBuffer.registerHandler(handler);
    }

    public FutureIU(String category, String idKey, String idVal)
    {
        this(category, idKey, idVal, new InputBuffer("FutureIU", ImmutableSet.of(category)));
    }

    /**
     * Closes the FutureIU, use only if get is not used.
     */
    public void cleanup()
    {
        inBuffer.removeHandler(handler);
        if (inBuffer.getOwningComponentName().equals("FutureIU"))
        {
            inBuffer.close();
        }
    }

    /**
     * Waits (if necessary) for the IU and take it (can be done only once)
     */
    public AbstractIU take() throws InterruptedException
    {
        AbstractIU iu;
        try
        {
            iu = queue.take();
        }
        finally
        {
            cleanup();
        }
        return iu;
    }

    /**
     * Wait for at most the given time for the IU and take it (can be done only once), return null on timeout
     */
    public AbstractIU take(long timeout, TimeUnit unit) throws InterruptedException
    {
        AbstractIU iu;
        try
        {
            iu = queue.poll(timeout, unit);
        }
        finally
        {
            cleanup();
        }
        return iu;
    }
}
