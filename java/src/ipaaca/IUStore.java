package ipaaca;

import java.util.HashMap;

/**
 * An IUStore maps an IUid to an IU
 * @author hvanwelbergen
 *
 * @param <X> type of AbstractIU stored in the store
 */
public class IUStore<X extends AbstractIU> extends HashMap<String, X>
{
    private static final long serialVersionUID = 1L;
}
