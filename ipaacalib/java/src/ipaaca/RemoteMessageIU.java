package ipaaca;

import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.util.List;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.google.common.collect.SetMultimap;

@Slf4j
public class RemoteMessageIU extends AbstractIU
{

    public RemoteMessageIU(String uid)
    {
        super(uid);
        payload = new Payload(this);
    }

    @Override
    public void commit()
    {
        log.info("Info: committing to a RemoteMessage only has local effects");
        committed = true;
    }

    @Override
    public void commit(String writerName)
    {
        log.info("Info: committing to a RemoteMessage only has local effects");
        committed = true;        
    }

    @Override
    void setPayload(List<PayloadItem> newItems, String writerName)
    {
        for(PayloadItem item:newItems)
        {
            payload.put(item.getKey(),item.getValue());
        }
        log.info("Info: modifying a RemoteMessage only has local effects");
    }

    @Override
    void putIntoPayload(String key, String value, String writer)
    {
        payload.put(key,value);
        log.info("Info: modifying a RemoteMessage only has local effects");
    }
    
    void putIntoPayload(Map<? extends String, ? extends String> newItems, String writer) {
    	for (Map.Entry<? extends String, ? extends String> item : newItems.entrySet())
    	{
    		payload.put(item.getKey(), item.getValue());
    	    //System.out.println(entry.getKey() + "/" + entry.getValue());
    	}
    	log.info("Info: modifying a RemoteMessage only has local effects");
    }

    @Override
    void removeFromPayload(Object key, String writer)
    {
        payload.remove(key);
        log.info("Info: modifying a RemoteMessage only has local effects");
    }

    @Override
    void handlePayloadSetting(List<PayloadItem> newPayload, String writerName)
    {
                
    }

    @Override
    void modifyLinks(boolean isDelta, SetMultimap<String, String> linksToAdd, SetMultimap<String, String> linksToRemove, String Writer)
    {
        log.info("Info: modifying a RemoteMessage only has local effects");        
    }
    
}
