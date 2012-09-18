package ipaaca;

import java.util.ArrayList;
import java.util.List;

/**
 * Stores ius for which add messages occured.
 * @author hvanwelbergen
 * 
 */
public class StoringEventHandler implements HandlerFunctor
{
    private List<AbstractIU> addedIUs = new ArrayList<>();

    public List<AbstractIU> getAddedIUs()
    {
        return addedIUs;
    }

    @Override
    public void handle(AbstractIU iu, IUEventType type, boolean local)
    {
        switch (type)
        {
        case ADDED:
            addedIUs.add(iu);
            break;
        case COMMITTED:
            break;
        case UPDATED:
            break;
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

}
