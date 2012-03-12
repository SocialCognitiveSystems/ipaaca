package ipaacademo;

import java.util.EnumSet;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import ipaaca.*;

import javax.swing.JFrame;
import javax.swing.JLabel;

import com.google.common.collect.ImmutableSet;

public class TextPrinter
{
    
    private static final class MyEventHandler implements HandlerFunctor
    {
        @Override
        public void handle(AbstractIU iu, IUEventType type, boolean local)
        {
            switch(type)
            {
            case ADDED:  System.out.println("IU added "+iu.getPayload());  break;
            case COMMITTED: System.out.println("IU committed");  break;
            case UPDATED: System.out.println("IU updated "+iu.getPayload()); break;
            case LINKSUPDATED: System.out.println("IU links updated"); break;
            }            
        }
        
    }
    
    static
    {
        Initializer.initializeIpaacaRsb();
    }
    
    private static final String CATEGORY = "TEXT";
    private static final double RATE = 0.5;
    private UpdateThread updateThread;
    
    public TextPrinter()
    {
        Set<String> categories = new ImmutableSet.Builder<String>().add(CATEGORY).build();    
        JLabel label = new JLabel("");
        
        updateThread = new UpdateThread(new InputBuffer("TextPrinter", categories),label);
        
        JFrame frame = new JFrame("IPAACA TextPrinter Demo");
        frame.add(label);
        frame.setSize(1000,300);
        frame.setVisible(true);   
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
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
            long startTime = System.currentTimeMillis();
            long nextTime = System.currentTimeMillis();
            //long lastNewLetterTime = System.currentTimeMillis();
            while(true)
            {
                //double duration = (System.currentTimeMillis()-startTime)/1000d;
                long currentTime = System.currentTimeMillis();

                RemotePushIU iuFirst = null;
                for (RemotePushIU iu : inBuffer.getIUs())
                {
                    if(iu.getLinks("PREDECESSOR").isEmpty())
                    {
                        iuFirst = iu;
                        break;
                    }
                }
                if (iuFirst==null) {
                    startTime = System.currentTimeMillis();
                    continue;
                }
                //int numChars = (int)(duration/RATE);

                if (currentTime >= nextTime) {
                    AbstractIU iu = iuFirst;
                    String str = "";
                    int i = 0;
                    boolean first_new_element = true;
                    do {
                        if (iu.getPayload().get("STATE") != null || first_new_element) {
                            str += "<font color=\"#000000\">";
                            str += iu.getPayload().get("CONTENT");
                            str += "</font>";
                            if (iu.getPayload().get("STATE") == null) {
                                iu.getPayload().put("STATE", "REALIZED");
                                first_new_element = false;
                                nextTime = currentTime + (int)(1000.0*RATE);
                            }
                        } else {
                            str += "<font color=\"#909090\">";
                            str += iu.getPayload().get("CONTENT");
                            str += "</font>";
                            //lastNewLetterTime = System.currentTimeMillis();
                        }
                        Set<String> successor = iu.getLinks("SUCCESSOR");
                        if(successor!=null && !successor.isEmpty())
                        {
                            iu = inBuffer.getIU(successor.iterator().next());
                        }
                        i++;
                    } while(! iu.getLinks("SUCCESSOR").isEmpty());
                    if (iu.getPayload().get("STATE") != null || first_new_element) {
                    //if (i<numChars) {
                        str += "<font color=\"#000000\">";
                        str += iu.getPayload().get("CONTENT");
                        str += "</font>";
                        if (iu.getPayload().get("STATE") == null) {
                            iu.getPayload().put("STATE", "REALIZED");
                            first_new_element = false;
                            nextTime = currentTime + (int)(RATE*1000.0);
                        }
                    } else {
                        str += "<font color=\"#909090\">";
                        str += iu.getPayload().get("CONTENT");
                        str += "</font>";
                        //lastNewLetterTime = System.currentTimeMillis();
                    }

                    str = "<html>"+str+"</html>";
                    //System.out.println(str);
                    label.setText(str);
                } else {
                    // just wait until we can print some more
                }

                try
                {
                    Thread.sleep(200);
                }
                catch (InterruptedException e)
                {
                    Thread.interrupted();
                }
            }
        }
        
    }
    
    public static void main(String args[])
    {
        TextPrinter tp = new TextPrinter();        
        
        
        OutputBuffer outBuffer = new OutputBuffer("componentX");
        /*String[] inputString = {"h","e","l","l","o"," ","w","o","r","l","d","!"};
        LocalIU predIU = null;        
        for(String str:inputString)
        {
            LocalIU localIU = new LocalIU();
            localIU.setCategory(CATEGORY);
            outBuffer.add(localIU);
            localIU.getPayload().put("CONTENT", str);            
            if(predIU!=null)
            {
                localIU.setLinks("PREDECESSOR", ImmutableSet.of(predIU.getUid()));
                predIU.setLinks("SUCCESSOR",ImmutableSet.of(localIU.getUid()));
            }
            predIU = localIU;
        }*/

        tp.start();        
    }
}
