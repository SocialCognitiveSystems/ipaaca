/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2013 Sociable Agents Group
 *                         CITEC, Bielefeld University   
 *
 * http://opensource.cit-ec.de/projects/ipaaca/
 * http://purl.org/net/ipaaca
 *
 * This file may be licensed under the terms of of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
 *
 * The development of this software was supported by the
 * Excellence Cluster EXC 277 Cognitive Interaction Technology.
 * The Excellence Cluster EXC 277 is a grant of the Deutsche
 * Forschungsgemeinschaft (DFG) in the context of the German
 * Excellence Initiative.
 */

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
