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

package ipaaca;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * Base class for InputBuffer and OutputBuffer.
 */
public abstract class Buffer
{
    private final String owningComponentName;

    private List<IUEventHandler> eventHandlers = new ArrayList<IUEventHandler>();
    protected final String uuid = UUID.randomUUID().toString().replaceAll("-", "").substring(0, 8);
    protected String uniqueName;

    public String getUniqueShortName()
    {
    	return owningComponentName + "ID" + uuid;
    }
    
    public String getUniqueName()
    {
        return uniqueName;
    }

    public String getOwningComponentName()
    {
        return owningComponentName;
    }

    // def __init__(self, owning_component_name, participant_config=None):
    // '''Create a Buffer.
    //
    // Keyword arguments:
    // owning_compontent_name --
    // participant_config -- RSB configuration
    // '''
    // super(Buffer, self).__init__()
    // self._owning_component_name = owning_component_name
    // self._participant_config = participant_config #rsb.ParticipantConfig.fromDefaultSources() if participant_config is None else participant_config
    // self._uuid = str(uuid.uuid4())[0:8]
    // # Initialise with a temporary, but already unique, name
    // self._unique_name = "undef-"+self._uuid
    // self._iu_store = IUStore()
    // self._iu_event_handlers = []
    /**
     * @param owningComponentName name of the entity that owns this Buffer
     * @param participantConfig RSB configuration
     */
    public Buffer(String owningComponentName)
    {
        this.owningComponentName = owningComponentName;
        uniqueName = "undef-" + uuid;
    }

    // def register_handler(self, handler_function, for_event_types=None, for_categories=None):
    // """Register a new IU event handler function.
    //
    // Keyword arguments:
    // handler_function -- a function with the signature (IU, event_type, local)
    // for_event_types -- a list of event types or None if handler should
    // be called for all event types
    // for_categories -- a list of category names or None if handler should
    // be called for all categoires
    //
    // """
    // handler = IUEventHandler(handler_function=handler_function, for_event_types=for_event_types, for_categories=for_categories)
    // self._iu_event_handlers.append(handler)

    public void registerHandler(IUEventHandler handler)
    {
        eventHandlers.add(handler);
    }
   
    public void registerHandler(HandlerFunctor func) {
    	IUEventHandler handler;
    	handler = new IUEventHandler(func);
    	registerHandler(handler);
    }
    
    public void registerHandler(HandlerFunctor func, Set<String> categories) {
    	IUEventHandler handler;
    	handler = new IUEventHandler(func, categories);
    	registerHandler(handler);
    }
    
    public void registerHandler(HandlerFunctor func, EnumSet<IUEventType> eventTypes) {
    	IUEventHandler handler;
    	handler = new IUEventHandler(func, eventTypes);
    	registerHandler(handler);
    }
   
    public void registerHandler(HandlerFunctor func, EnumSet<IUEventType> eventTypes, Set<String> categories) {
    	IUEventHandler handler;
    	handler = new IUEventHandler(func, eventTypes, categories);
    	registerHandler(handler);
    }

    // def call_iu_event_handlers(self, uid, local, event_type, category):
    // """Call registered IU event handler functions registered for this event_type and category."""
    // for h in self._iu_event_handlers:
    // # print('calling an update handler for '+event_type+' -> '+str(h))
    // h.call(self, uid, local=local, event_type=event_type, category=category)
    /**
     * Call registered IU event handler functions registered for this event_type and category.
     */
    protected void callIuEventHandlers(String uid, boolean local, IUEventType type, String category)
    {
        for (IUEventHandler h : eventHandlers)
        {
            h.call(this, uid, local, type, category);
        }
    }

    public abstract AbstractIU getIU(String iuid);
}
