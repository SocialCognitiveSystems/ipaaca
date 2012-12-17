package ipaaca;

import static org.junit.Assert.assertNotNull;

import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import rsb.Factory;
import rsb.Informer;
import rsb.RSBException;

import com.google.common.collect.ImmutableSet;

/**
 * Unit testcases for the input buffer
 * @author hvanwelbergen
 *
 */
public class InputBufferTest
{
    private static final String COMPID = "Comp1";
    private static final String CATEGORY = "category1";
    private InputBuffer inBuffer;


    static
    {
        Initializer.initializeIpaacaRsb();
    }
    
    @Before
    public void setup()
    {
        Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();
        inBuffer = new InputBuffer(COMPID, categories);        
    }

    @After
    public void tearDown()
    {
        inBuffer.close();
    }
    
    @Test
    public void testHandleRemotePushEvent() throws RSBException, InterruptedException
    {
        Informer<Object> informer = Factory.getInstance().createInformer("/ipaaca/category/"+CATEGORY);
        informer.activate();
        RemotePushIU iu = new RemotePushIU("uid1");
        iu.setCategory("/ipaaca/category/"+CATEGORY);
        iu.setOwnerName("owner");
        iu.setReadOnly(false);
        iu.setRevision(1);
        informer.send(iu);
        
        Thread.sleep(1000);
        AbstractIU iuIn = inBuffer.getIU("uid1");
        assertNotNull(iuIn);
        informer.deactivate();
    }
}
