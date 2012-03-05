package ipaaca;

import static org.hamcrest.collection.IsIterableContainingInAnyOrder.containsInAnyOrder;
import static org.junit.Assert.*;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Set;

import org.junit.Before;
import org.junit.Test;

import com.google.common.collect.ImmutableSet;

public class JavaPythonTest
{

    static
    {
        Initializer.initializeIpaacaRsb();
    }

    private InputBuffer inBuffer;

    private static final String PYTHON_PREAMBLE = "import sys\n" 
            + "sys.path.insert(0, '../python/build/')\n"
            + "sys.path.insert(0, '../python/lib/')\n" 
            + "import ipaaca, time\n";

    @Before
    public void setup()
    {
        Set<String> categories = new ImmutableSet.Builder<String>().add("JavaPythonTest").build();
        inBuffer = new InputBuffer("javaside", categories);
    }

    private void printRuntimeErrors(Process p) throws IOException
    {

        InputStream in = p.getInputStream();
        BufferedInputStream buf = new BufferedInputStream(in);
        InputStreamReader inread = new InputStreamReader(buf);
        BufferedReader bufferedreader = new BufferedReader(inread);
        // Read the ls output
        String line;
        while ((line = bufferedreader.readLine()) != null)
        {
            System.out.println(line);
        }

        try
        {
            if (p.waitFor() != 0)
            {
                System.err.println("exit value = " + p.exitValue());
            }
        }
        catch (InterruptedException e)
        {
            System.err.println(e);
        }

        in = p.getErrorStream();
        buf = new BufferedInputStream(in);
        inread = new InputStreamReader(buf);
        bufferedreader = new BufferedReader(inread);
        // Read the ls output
        while ((line = bufferedreader.readLine()) != null)
        {
            System.out.println(line);
        }
    }

    private boolean runPythonProgram(String pypr) throws IOException
    {
        Process p = Runtime.getRuntime().exec(new String[] { "python", "-c", pypr });
        printRuntimeErrors(p);
        return p.exitValue()==0;
    }

    @Test
    public void testSetPayloadInPythonOutputBuffer() throws IOException, InterruptedException
    {

        String pypr = PYTHON_PREAMBLE 
                + "ob = ipaaca.OutputBuffer('pythonside')\n" 
                + "iu = ipaaca.IU('JavaPythonTest')\n"
                + "iu.payload = {'data':'Hello from Python!'}\n" 
                + "time.sleep(0.1)\n" 
                + "ob.add(iu)\n";
        assertTrue(runPythonProgram(pypr));

        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertEquals("Hello from Python!", iu.getPayload().get("data"));
    }
    
    @Test
    public void testSetPayloadInPythonOutputBufferAfterPublishing() throws IOException, InterruptedException
    {

        String pypr = PYTHON_PREAMBLE 
                + "ob = ipaaca.OutputBuffer('pythonside')\n" 
                + "iu = ipaaca.IU('JavaPythonTest')\n"                
                + "ob.add(iu)\n"
                + "time.sleep(0.1)\n"
                + "iu.payload = {'data':'Hello from Python!'}\n";
                
        assertTrue(runPythonProgram(pypr));

        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertEquals("Hello from Python!", iu.getPayload().get("data"));
    }
    
    @Test
    public void testAddLinkThenPublishInPython() throws IOException, InterruptedException
    {
        String pypr = PYTHON_PREAMBLE 
                +"ob = ipaaca.OutputBuffer('pythonside')\n"
                +"iu = ipaaca.IU('JavaPythonTest')\n"
                +"iu.add_links('testtype',['dummy1','dummy2'])\n"
                + "time.sleep(0.1)\n" 
                + "ob.add(iu)\n";
        assertTrue(runPythonProgram(pypr));
        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertThat(iu.getLinks("testtype"),containsInAnyOrder("dummy1","dummy2"));       
        
    }
    
    @Test
    public void testPublishThenAddLinkInPython() throws IOException, InterruptedException
    {
        String pypr = PYTHON_PREAMBLE 
                +"ob = ipaaca.OutputBuffer('pythonside')\n"
                +"iu = ipaaca.IU('JavaPythonTest')\n"
                + "ob.add(iu)\n"
                + "time.sleep(0.1)\n"        
                +"iu.add_links('testtype',['dummy1','dummy2'])\n";
                
        assertTrue(runPythonProgram(pypr));
        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertThat(iu.getLinks("testtype"),containsInAnyOrder("dummy1","dummy2"));        
    }
    
    @Test
    public void testCommitPublishedIUFromPython()throws IOException, InterruptedException
    {
        String pypr = PYTHON_PREAMBLE 
                +"ob = ipaaca.OutputBuffer('pythonside')\n"
                +"iu = ipaaca.IU('JavaPythonTest')\n"
                + "ob.add(iu)\n"
                + "time.sleep(0.1)\n"
                + "iu.commit()\n";
        assertTrue(runPythonProgram(pypr));
        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertTrue(iu.isCommitted());
    }
    
    @Test
    public void testCommitThenPublishIUFromPython()throws IOException, InterruptedException
    {
        String pypr = PYTHON_PREAMBLE 
                +"ob = ipaaca.OutputBuffer('pythonside')\n"
                +"iu = ipaaca.IU('JavaPythonTest')\n"
                +"iu.commit()\n"
                +"time.sleep(0.1)\n"
                +"ob.add(iu)\n";
                
        assertTrue(runPythonProgram(pypr));
        Thread.sleep(200);
        assertEquals(1, inBuffer.getIUs().size());
        AbstractIU iu = inBuffer.getIUs().iterator().next();
        assertTrue(iu.isCommitted());
    }
}
