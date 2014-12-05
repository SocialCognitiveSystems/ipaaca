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

import ipaaca.AbstractIU;
import ipaaca.HandlerFunctor;
import ipaaca.IUEventHandler;
import ipaaca.IUEventType;
import ipaaca.Initializer;
import ipaaca.InputBuffer;
import ipaaca.OutputBuffer;
import ipaaca.RemotePushIU;

import java.util.EnumSet;
import java.util.Set;

import javax.swing.JFrame;
import javax.swing.JLabel;

import com.google.common.collect.ImmutableSet;

public class TestListener
{

    private static final class MyEventHandler implements HandlerFunctor
    {
        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            switch(type)
            {
            case ADDED:  System.out.println("IU added "+iu.getPayload().get("CONTENT"));  break;
            case COMMITTED: System.out.println("IU committed");  break;
            case UPDATED: System.out.println("IU updated "+iu.getPayload()); break;
            case LINKSUPDATED: System.out.println("IU links updated"); break;
            case RETRACTED: break;
            case DELETED: break;
            }
        }

    }

    static
    {
        Initializer.initializeIpaacaRsb();
    }

    private static final String CATEGORY = "spam";
    private static final double RATE = 0.5;
    private UpdateThread updateThread;

    public TestListener()
    {
        Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();
        JLabel label = new JLabel("");

        updateThread = new UpdateThread(new InputBuffer("TestListener", categories),label);
    }

    public void start()
    {
        updateThread.start();
    }

    private static class UpdateThread extends Thread
    {
        private InputBuffer inBuffer;
        private JLabel label;

        public UpdateThread(InputBuffer inBuffer, JLabel label)
        {
            this.inBuffer = inBuffer;
            this.label = label;

            EnumSet<IUEventType> types = EnumSet.of(IUEventType.ADDED,IUEventType.COMMITTED,IUEventType.UPDATED,IUEventType.LINKSUPDATED);
            Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();
            MyEventHandler printingEventHandler;
            printingEventHandler = new MyEventHandler();
            this.inBuffer.registerHandler(new IUEventHandler(printingEventHandler,types,categories));
        }

        @Override
        public void run()
        {
        	System.out.println("Starting!");
        }

    }

    public static void main(String args[])
    {
        TestListener tl = new TestListener();
        tl.start();
    }
}
