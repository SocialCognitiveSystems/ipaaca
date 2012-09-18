package ipaaca;

import java.util.HashMap;
import java.util.Map;

/**
 * Counts how often and in what order certain events occur
 * @author hvanwelbergen
 *
 */
final class CountingEventHandler implements HandlerFunctor
{
    private Map<String,Integer> commitEvents = new HashMap<String,Integer>();
    private Map<String,Integer> addEvents = new HashMap<String,Integer>();
    private Map<String,Integer> updateEvents = new HashMap<String,Integer>();
    
    private void updateEventMap(String key, Map<String,Integer> map)
    {
        int value = 0;
        if(map.containsKey(key))
        {
            value = map.get(key);
        }
        value++;
        map.put(key, value);
    }
    
    @Override
    public void handle(AbstractIU iu, IUEventType type, boolean local)
    {
        switch(type)
        {
        case ADDED:  updateEventMap(iu.getUid(),addEvents);  break;
        case COMMITTED: updateEventMap(iu.getUid(),commitEvents); break;
        case UPDATED: updateEventMap(iu.getUid(),updateEvents); break;
        case DELETED:
            break;
        case LINKSUPDATED:
            break;
        case RETRACTED:
            break;
        default:
            break;
        }            
    }
    
    public int getNumberOfCommitEvents(String iu)
    {
        if(!commitEvents.containsKey(iu))
        {
            return 0;
        }
        return commitEvents.get(iu);
    }
    
    public int getNumberOfAddEvents(String iu)
    {
        if(!addEvents.containsKey(iu))
        {
            return 0;
        }
        return addEvents.get(iu);
    }
    
    public int getNumberOfUpdateEvents(String iu)
    {
        if(!updateEvents.containsKey(iu))
        {
            return 0;
        }
        return updateEvents.get(iu);
    }
}