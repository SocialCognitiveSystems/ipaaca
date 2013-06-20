package ipaaca;

import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

/**
 * Wrapper for IU event handling functions.
 * @author hvanwelbergen
 */
public class IUEventHandler
{
    private final EnumSet<IUEventType> eventTypes;
    private Set<String> categories;
    private final HandlerFunctor handleFunctor;

    // def __init__(self, handler_function, for_event_types=None, for_categories=None):
    // """Create an IUEventHandler.
    //
    // Keyword arguments:
    // handler_function -- the handler function with the signature
    // (IU, event_type, local)
    // for_event_types -- a list of event types or None if handler should
    // be called for all event types
    // for_categories -- a list of category names or None if handler should
    // be called for all categoires
    // """
    // super(IUEventHandler, self).__init__()
    // self._handler_function = handler_function
    // self._for_event_types = (
    // None if for_event_types is None else
    // (for_event_types[:] if hasattr(for_event_types, '__iter__') else [for_event_types]))
    // self._for_categories = (
    // None if for_categories is None else
    // (for_categories[:] if hasattr(for_categories, '__iter__') else [for_categories]))
    
    public IUEventHandler(HandlerFunctor func)
    {
        this.handleFunctor = func;
        this.categories = new HashSet<String>();
        this.eventTypes = EnumSet.allOf(IUEventType.class);
    }
    
    public IUEventHandler(HandlerFunctor func, Set<String> categories)
    {
        this.handleFunctor = func;
        this.categories = categories;
        this.eventTypes = EnumSet.allOf(IUEventType.class);
    }
    
    public IUEventHandler(HandlerFunctor func, EnumSet<IUEventType> eventTypes)
    {
        this.handleFunctor = func;
        this.eventTypes = eventTypes;
        this.categories = new HashSet<String>();
    }

    public IUEventHandler(HandlerFunctor func, EnumSet<IUEventType> eventTypes, Set<String> categories)
    {
        this.eventTypes = eventTypes;
        this.categories = categories;
        this.handleFunctor = func;
    }

    // def condition_met(self, event_type, category):
    // """Check whether this IUEventHandler should be called.
    //
    // Keyword arguments:
    // event_type -- type of the IU event
    // category -- category of the IU which triggered the event
    // """
    // type_condition_met = (self._for_event_types is None or event_type in self._for_event_types)
    // cat_condition_met = (self._for_categories is None or category in self._for_categories)
    // return type_condition_met and cat_condition_met
    /**
     * Check whether this IUEventHandler should be called.
     * @param type type of the IU event
     * @param category category of the IU which triggered the event
     */
    private boolean conditionMet(IUEventType type, String category)
    {
    	if (this.categories.isEmpty()) { // match any category
    		return this.eventTypes.contains(type);
    	}
    	else
    	{
    		return this.eventTypes.contains(type) && this.categories.contains(category);
    	}
    }

    // def call(self, buffer, iu_uid, local, event_type, category):
    // """Call this IUEventHandler's function, if it applies.
    //
    // Keyword arguments:
    // buffer -- the buffer in which the IU is stored
    // iu_uid -- the uid of the IU
    // local -- is the IU local or remote to this component? @RAMIN: Is this correct?
    // event_type -- IU event type
    // category -- category of the IU
    // """
    // if self.condition_met(event_type, category):
    // iu = buffer._iu_store[iu_uid]
    // self._handler_function(iu, event_type, local)
    public void call(Buffer buf, String iuUid, boolean local, IUEventType type, String category)
    {
        if (conditionMet(type, category))
        {
            AbstractIU iu = buf.getIU(iuUid);
            handleFunctor.handle(iu, type, local);
        }
    }
}
