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
