package ipaaca.util;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventHandler;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;
import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import com.google.common.base.Joiner;
import com.google.common.collect.ImmutableSet;

/**
 * Utility class to handle component notification: a componentNotify is sent at initialization and whenever new components sent
 * their componentNotify.
 * @author hvanwelbergen
 * 
 */
public class ComponentNotifier
{
    public static final String NOTIFY_CATEGORY = "componentNotify";
    public static final String SEND_CATEGORIES = "send_categories";
    public static final String RECEIVE_CATEGORIES = "recv_categories";
    public static final String STATE = "state";
    public static final String NAME = "name";
    private final OutputBuffer outBuffer;
    private final String componentName;
    private final String componentFunction;
    private final ImmutableSet<String> sendCategories;
    private final ImmutableSet<String> receiveCategories;
    private final InputBuffer inBuffer;
    private List<HandlerFunctor> handlers = new ArrayList<HandlerFunctor>();

    private class ComponentNotifyHandler implements HandlerFunctor
    {
        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            if(iu.getPayload().get(NAME).equals(componentName))return; //don't re-notify self
            for (HandlerFunctor h : handlers)
            {
                h.handle(iu, type, local);
            }
            if (iu.getPayload().get(STATE).equals("new"))
            {
                submitNotify(false);
            }
        }
    }

    /**
     * Register a handler that will be called whenever a new component notifies this ComponentNotifier
     */
    public void addNotificationHandler(HandlerFunctor h)
    {
        handlers.add(h);
    }

    private void submitNotify(boolean isNew)
    {
        LocalMessageIU notifyIU = new LocalMessageIU();
        notifyIU.setCategory(NOTIFY_CATEGORY);
        notifyIU.getPayload().put(NAME, componentName);
        notifyIU.getPayload().put("function", componentFunction);
        notifyIU.getPayload().put(SEND_CATEGORIES, Joiner.on(",").join(sendCategories));
        notifyIU.getPayload().put(RECEIVE_CATEGORIES, Joiner.on(",").join(receiveCategories));
        notifyIU.getPayload().put(STATE, isNew ? "new" : "old");
        outBuffer.add(notifyIU);
    }

    public ComponentNotifier(String componentName, String componentFunction, Set<String> sendCategories, Set<String> receiveCategories,
            OutputBuffer outBuffer, InputBuffer inBuffer)
    {
        this.componentName = componentName;
        this.componentFunction = componentFunction;
        this.sendCategories = ImmutableSet.copyOf(sendCategories);
        this.receiveCategories = ImmutableSet.copyOf(receiveCategories);
        this.outBuffer = outBuffer;
        this.inBuffer = inBuffer;
    }

    public void initialize()
    {
        inBuffer.registerHandler(new IUEventHandler(new ComponentNotifyHandler(), EnumSet.of(IUEventType.ADDED), ImmutableSet
                .of(NOTIFY_CATEGORY)));
        submitNotify(true);
    }
}
