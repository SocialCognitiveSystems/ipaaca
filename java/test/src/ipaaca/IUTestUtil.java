package ipaaca;
import static org.hamcrest.collection.IsIterableContainingInAnyOrder.containsInAnyOrder;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;
import ipaaca.Ipaaca.IU;
import ipaaca.Ipaaca.LinkSet;
import ipaaca.Ipaaca.PayloadItem;

import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;

import com.google.common.collect.SetMultimap;

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
        assertEquals(iu1.getPayload(),iu2.getPayload());
        assertEquals(iu1.getAllLinks(),iu2.getAllLinks());
    }
    
    public static void assertEqualPayload(Collection<PayloadItem> pl1, Payload pl2)
    {
        // all payload items in pl1 are in pl2
        for (PayloadItem item : pl1)
        {
            assertTrue("Key "+item.getKey() +" from protocol buffer payload not found in IU payload "+pl2,
                    pl2.containsKey(item.getKey()));
            assertEquals(item.getValue(), pl2.get(item.getKey()));
        }

        // all payload items in pl12 are in pl1
        for (Entry<String, String> entry : pl2.entrySet())
        {
            boolean found = false;
            for (PayloadItem item : pl1)
            {
                if (item.getKey().equals(entry.getKey()) && item.getValue().equals(entry.getValue()))
                {
                    found = true;
                    break;
                }
            }
            assertTrue("Entry " + entry + "from IU implementation not found in protocol buffer. Protocal buffer payload: "
                    + pl2,found);
        }
    }
    
    public static void assertEqualLinks(List<LinkSet> links1, SetMultimap<String,String> links2)
    {
        // all links in links1 are in links2
        for(LinkSet ls:links1)
        {
            assertThat(links2.get(ls.getType()),containsInAnyOrder(ls.getTargetsList().toArray(new String[0])));            
        }
        
        // all links in links2 are in links1
        for(Entry<String, Collection<String>> entry:links2.asMap().entrySet())
        {
            boolean found = false;
            for(LinkSet ls:links1)
            {
                if(ls.getType().equals(entry.getKey()))
                {
                    assertThat(entry.getValue(), containsInAnyOrder(ls.getTargetsList().toArray(new String[0])));            
                    found = true;
                    break;
                }
            }
            assertTrue("Link " + entry + "from IU implementation not found in protocol buffer. Protocal buffer links: "
                    + links2,found);
        }
    }
    
    public static void assertEqualIU(IU iuSrc, AbstractIU iuTarget)
    {
        assertEquals(iuSrc.getCategory(), iuTarget.getCategory());
        assertEquals(iuSrc.getCommitted(), iuTarget.isCommitted());
        assertEquals(iuSrc.getUid(), iuTarget.getUid());
        assertEquals(iuSrc.getRevision(), iuTarget.getRevision());
        assertEquals(iuSrc.getReadOnly(), iuTarget.isReadOnly());
        assertEqualPayload(iuSrc.getPayloadList(),iuTarget.getPayload());
        assertEqualLinks(iuSrc.getLinksList(),iuTarget.getAllLinks());                
    }
}
