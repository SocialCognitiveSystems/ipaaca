package ipaaca;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Base class for InputBuffer and OutputBuffer.
 */
public abstract class Buffer
{
    private final String owningComponentName;

    private List<IUEventHandler> eventHandlers = new ArrayList<IUEventHandler>();
    protected final String uuid = UUID.randomUUID().toString().replaceAll("-", "");
    protected String uniqueName;

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

    // def call_iu_event_handlers(self, uid, local, event_type, category):
    // """Call registered IU event handler functions registered for this event_type and category."""
    // for h in self._iu_event_handlers:
    // # print('calling an update handler for '+event_type+' -> '+str(h))
    // h.call(self, uid, local=local, event_type=event_type, category=category)
    /**
     * Call registered IU event handler functions registered for this event_type and category.
     */
    public void callIuEventHandlers(String uid, boolean local, IUEventType type, String category)
    {
        for (IUEventHandler h : eventHandlers)
        {
            h.call(this, uid, local, type, category);
        }
    }

    public abstract AbstractIU getIU(String iuid);
}
