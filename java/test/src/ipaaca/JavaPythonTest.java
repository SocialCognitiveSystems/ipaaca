package ipaaca;

import static org.junit.Assert.assertEquals;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Set;

import org.junit.Before;
import org.junit.Test;

import com.google.common.collect.ImmutableSet;

public class JavaPythonTest {
	
    static {
        Initializer.initializeIpaacaRsb();
    }

    private InputBuffer inBuffer;

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
	
	@Test
	public void test() throws IOException, InterruptedException {
		
		String pypr = 
		"import sys\n" +
		"sys.path.append('../python/build/')\n" +
		"sys.path.append('../python/lib/')\n" +
		"import ipaaca, time\n" +
		"ob = ipaaca.OutputBuffer('pythonside')\n" + 
		"iu = ipaaca.IU('JavaPythonTest')\n" +
		"iu.payload = {'data':'Hello from Python!'}\n" +
		"time.sleep(0.1)\n" +
		"ob.add(iu)";
		Process p = Runtime.getRuntime().exec(new String[]{"python","-c", pypr});
		printRuntimeErrors(p);
		
		Thread.sleep(200);
		assertEquals(1, inBuffer.getIUs().size());
		AbstractIU iu = inBuffer.getIUs().iterator().next();
		assertEquals("Hello from Python!",iu.getPayload().get("data"));
	}
	
	
}
