package ipaacademo;

import java.util.Set;

import ipaaca.AbstractIU;
import ipaaca.Initializer;
import ipaaca.InputBuffer;
import ipaaca.LocalIU;
import ipaaca.OutputBuffer;
import ipaaca.RemotePushIU;

import javax.swing.JFrame;
import javax.swing.JLabel;

import com.google.common.collect.ImmutableSet;

public class TextPrinter
{
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
        }
        
        @Override
        public void run()
        {
            long startTime = System.currentTimeMillis();
            while(true)
            {
                double duration = (System.currentTimeMillis()-startTime)/1000d;
                
                RemotePushIU iuFirst = null;
                for (RemotePushIU iu : inBuffer.getIUs())
                {
                    if(iu.getLinks("PREDECESSOR").isEmpty())
                    {
                        iuFirst = iu;
                        break;
                    }
                }
                
                int numChars = (int)(duration/RATE);
                
                AbstractIU iu = iuFirst;
                String str = "";
                for(int i=0;i<numChars;i++)
                {
                    str += iu.getPayload().get("CONTENT");
                    Set<String> successor = iu.getLinks("SUCCESSOR");
                    if(successor!=null && !successor.isEmpty())
                    {
                        iu = inBuffer.getIU(successor.iterator().next());
                    }
                    else
                    {
                        break;
                    }
                }
                label.setText(str);
                
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
        String[] inputString = {"h","e","l","l","o"," ","w","o","r","l","d","!"};
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
        }
        tp.start();        
    }
}
