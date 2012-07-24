package ipaacademo;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * Demonstrates how to call a python script from java
 * @author hvanwelbergen
 *
 */
public class PythonCall
{

    /**
     * @param args
     * @throws IOException
     */
    public static void main(String[] args) throws IOException
    {
        String program = "print 'Hello world'";
        Process p = Runtime.getRuntime().exec(new String[]{"python","-c", program});
        
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
}
