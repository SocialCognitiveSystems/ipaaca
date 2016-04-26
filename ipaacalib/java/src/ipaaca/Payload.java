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

import org.apache.commons.lang.StringEscapeUtils;

import com.google.common.collect.ImmutableSet;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Payload of an IU
 * @author hvanwelbergen
 * 
 */
public class Payload implements Map<String, String>
{
    private Map<String, String> map = Collections.synchronizedMap(new HashMap<String, String>());
    private final AbstractIU iu;

    public Payload(AbstractIU iu)
    {
        this.iu = iu;
    }

    public Payload(AbstractIU iu, List<PayloadItem> payloadItems)
    {
        this(iu, payloadItems, null);
    }

    public Payload(AbstractIU iu, Map<String, String> newPayload)
    {
        this(iu, newPayload, null);
    }

    public Payload(AbstractIU iu, Map<String, String> newPayload, String writerName)
    {
        this.iu = iu;
        set(newPayload, writerName);
    }

    public Payload(AbstractIU iu, List<PayloadItem> newPayload, String writerName)
    {
        this.iu = iu;
        set(newPayload, writerName);
    }

    public void set(Map<String, String> newPayload, String writerName)
    {
        iu.setPayload(newPayload, writerName);
        synchronized(map)
        {
            map.clear();
            map.putAll(newPayload);
        }
    }

    public void set(List<PayloadItem> newPayload, String writerName)
    {
        iu.handlePayloadSetting(newPayload, writerName);
        synchronized(map)
        {
            map.clear();
            for (PayloadItem item : newPayload)
            {
                map.put(item.getKey(), pseudoConvertFromJSON(item.getValue(), item.getType()));
            }
        }
    }

    public String pseudoConvertFromJSON(String value, String type) {
        if (type.equals("JSON")) {
            if (value.startsWith("\"")) {
                //return value.replaceAll("\\\"", "");
                return StringEscapeUtils.unescapeJava(value.substring(1, value.length() - 1));
            } else if (value.startsWith("{") || value.startsWith("[") || value.matches("true") || value.matches("false") || value.matches("-?[0-9]*[.,]?[0-9][0-9]*.*")) { 
                return value;
            } else if (value.equals("null")) {
                return "";
            }
        }
        return value;
    }

    void enforcedSetItem(String key, String value)
    {
        map.put(key, value);
    }

    void enforcedRemoveItem(String key)
    {
        map.remove(key);
    }

    public void clear()
    {
        throw new RuntimeException("Not implemented");
        // map.clear();
    }

    public boolean containsKey(Object key)
    {
        return map.containsKey(key);
    }

    public boolean containsValue(Object value)
    {
        return map.containsValue(value);
    }

    /**
     * Provides an immutable copy of the entryset of the Payload
     */
    public ImmutableSet<java.util.Map.Entry<String, String>> entrySet()
    {
        return ImmutableSet.copyOf(map.entrySet());
    }

    public boolean equals(Object o)
    {
        return map.equals(o);
    }

    public String get(Object key)
    {
        return map.get(key);
    }

    public int hashCode()
    {
        return map.hashCode();
    }

    public boolean isEmpty()
    {
        return map.isEmpty();
    }

    public Set<String> keySet()
    {
        return map.keySet();
    }

    /**
     * Set item in this payload.
     * Requests item setting from the OutputBuffer holding the local version
     * of this IU. Returns when permission is granted and item is set;
     * otherwise raises an IUUpdateFailedException.
     */
    public String put(String key, String value, String writer)
    {
        iu.putIntoPayload(key, value, writer);
        return map.put(key, value);
    }

    /**
     * Delete item in this payload.//
     * Requests item deletion from the OutputBuffer holding the local version
     * of this IU. Returns when permission is granted and item is deleted;
     * otherwise raises an IUUpdateFailedError.
     */
    public String remove(Object key, String writer)
    {
        iu.removeFromPayload(key, writer);
        return map.remove(key);
    }

    public String put(String key, String value)
    {
        return put(key, value, null);
    }

    
    public void putAll(Map<? extends String, ? extends String> newItems)
    {
        putAll(newItems, null);
    }

    public void putAll(Map<? extends String, ? extends String> newItems, String writer)
    {
        iu.putIntoPayload(newItems, writer);
   		map.putAll(newItems);
    }
    
    public void merge(Map<? extends String, ? extends String> items) {
    	putAll(items, null);
    }

    public String remove(Object key)
    {
        return remove(key, null);
    }

    public int size()
    {
        return map.size();
    }

    public Collection<String> values()
    {
        return map.values();
    }
    
    @Override
    public String toString()
    {
        return map.toString();
    }
}
