package ipaaca;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import ipaaca.Ipaaca.IU;
import ipaaca.Ipaaca.PayloadItem;

import java.util.Map.Entry;

/**
 * Test utilities for the IU system.
 * @author hvanwelbergen
 */
public final class IUTestUtil
{
    private IUTestUtil(){}
    
    public static void assertEqualIU(AbstractIU iu1, AbstractIU iu2)
    {
        assertEquals(iu1.getCategory(), iu2.getCategory());
        assertEquals(iu1.isCommitted(), iu2.isCommitted());
        assertEquals(iu1.getUid(), iu2.getUid());
        assertEquals(iu1.getRevision(), iu2.getRevision());
        assertEquals(iu1.isReadOnly(), iu2.isReadOnly());
        iu1.getPayload().equals(iu2.getPayload());
    }
    
    public static void assertEqualIU(IU iuSrc, AbstractIU iuTarget)
    {
        assertEquals(iuSrc.getCategory(), iuTarget.getCategory());
        assertEquals(iuSrc.getCommitted(), iuTarget.isCommitted());
        assertEquals(iuSrc.getUid(), iuTarget.getUid());
        assertEquals(iuSrc.getRevision(), iuTarget.getRevision());
        assertEquals(iuSrc.getReadOnly(), iuTarget.isReadOnly());

        // all payload items in src are in target
        for (PayloadItem item : iuSrc.getPayloadList())
        {
            assertTrue("Key "+item.getKey() +" from protocol buffer payload not found in IU payload "+iuTarget.getPayload(),
                    iuTarget.getPayload().containsKey(item.getKey()));
            assertEquals(item.getValue(), iuTarget.getPayload().get(item.getKey()));
        }

        // all payload items in target are in source
        for (Entry<String, String> entry : iuTarget.getPayload().entrySet())
        {
            boolean found = false;
            for (PayloadItem item : iuSrc.getPayloadList())
            {
                if (item.getKey().equals(entry.getKey()) && item.getValue().equals(entry.getValue()))
                {
                    found = true;
                    break;
                }
            }
            assertTrue("Entry " + entry + "from IU implementation not found in protocol buffer. Protocal buffer payload: "
                    + iuSrc.getPayloadList(),found);
        }
    }
}
