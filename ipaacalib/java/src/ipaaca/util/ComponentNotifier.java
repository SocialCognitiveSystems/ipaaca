/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2013 Sociable Agents Group
 *                         CITEC, Bielefeld University   
 *
 * http://opensource.cit-ec.de/projects/ipaaca/
 * http://purl.org/net/ipaaca
 *
 * This file may be licensed under the terms of of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
 *
 * The development of this software was supported by the
 * Excellence Cluster EXC 277 Cognitive Interaction Technology.
 * The Excellence Cluster EXC 277 is a grant of the Deutsche
 * Forschungsgemeinschaft (DFG) in the context of the German
 * Excellence Initiative.
 */

package ipaaca.util;

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventHandler;
import ipaaca.IUEventType;
import ipaaca.InputBuffer;
import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

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
    public static final String FUNCTION = "function";
    private final OutputBuffer outBuffer;
    private final String componentName;
    private final String componentFunction;
    private final ImmutableSet<String> sendCategories;
    private final ImmutableSet<String> receiveCategories;
    private final InputBuffer inBuffer;
    private List<HandlerFunctor> handlers = Collections.synchronizedList(new ArrayList<HandlerFunctor>());
    private volatile boolean isInitialized = false;
    private final BlockingQueue<String> receiverQueue = new LinkedBlockingQueue<String>();
    
    private class ComponentNotifyHandler implements HandlerFunctor
    {
        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            if(iu.getPayload().get(NAME).equals(componentName))return; //don't re-notify self
            String receivers[] = iu.getPayload().get(RECEIVE_CATEGORIES).split("\\s*,\\s*");
            receiverQueue.addAll(ImmutableSet.copyOf(receivers));
            synchronized(handlers)
            {
                for (HandlerFunctor h : handlers)
                {
                    h.handle(iu, type, local);
                }
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
    
    /**
     * Wait until the receivers are registered for categories
     */
    public void waitForReceivers(ImmutableSet<String> categories)
    {
        Set<String> unhandledCategories = new HashSet<String>(categories);        
        while(!unhandledCategories.isEmpty())
        {
            try
            {
                unhandledCategories.remove(receiverQueue.take());
                
            }
            catch (InterruptedException e)
            {
                Thread.interrupted();
            }            
        }
    }
    
    /**
     * Wait until receivers are registered for all categories sent by the component
     */
    public void waitForReceivers()
    {
        waitForReceivers(sendCategories);
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

    public synchronized void initialize()
    {
        if(!isInitialized)
        {
            inBuffer.registerHandler(new IUEventHandler(new ComponentNotifyHandler(), EnumSet.of(IUEventType.ADDED), ImmutableSet
                    .of(NOTIFY_CATEGORY)));
            submitNotify(true);
            isInitialized = true;
        }
    }
}
