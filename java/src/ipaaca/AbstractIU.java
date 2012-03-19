package ipaaca;

import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.SetMultimap;

/**
 * Skeleton implementation of Schlangen's incremental unit
 * @author hvanwelbergen
 * 
 */
public abstract class AbstractIU
{

    private Buffer buffer;
    protected String ownerName;
    protected Payload payload;
    protected String category;
    protected boolean committed = false;
    private String uid;
    protected int revision;
    private boolean readOnly = false;

    protected SetMultimap<String, String> links = HashMultimap.create();
    private final SetMultimap<String, String> EMPTYLINKS = HashMultimap.create();

    public SetMultimap<String, String> getAllLinks()
    {
        return links;
    }
    
    public Set<String> getLinks(String type)
    {
        return links.get(type);
    }

    public void setLinksLocally(SetMultimap<String, String> l)
    {
        links.clear();
        links.putAll(l);
    }
    
    public void setLinksLocally(String type, Set<String> values)
    {
        links.removeAll(type);
        links.putAll(type, values);        
    }
    
    /**
     * Replace all links
     */
    public void setLinks(SetMultimap<String, String> l)
    {
        links.clear();
        links.putAll(l);
        modifyLinks(false, l, EMPTYLINKS, null);
    }
    
    public void modifyLinks(SetMultimap<String, String> linksAdd, SetMultimap<String, String> linksRemove)
    {
        links.putAll(linksAdd);
        for (Entry<String, Collection<String>> entry: linksRemove.asMap().entrySet())
        {
            for (String value : entry.getValue())
            {
                links.remove(entry.getKey(), value);
            }
        }
        modifyLinks(true, linksAdd, linksRemove, null);
    }
    /**
     * Replace the links of type by a new set of links
     */
    public void setLinks(String type, Set<String> values)
    {
        links.removeAll(type);
        links.putAll(type, values);
        modifyLinks(false, links, EMPTYLINKS, null);
    }
    
    
    /**
     * Remove values from links with type type
     */
    public void removeLinks(String type, Set<String> values)
    {
        for (String value : values)
        {
            links.remove(type, value);
        }
        SetMultimap<String, String> removeLinks = HashMultimap.create();
        removeLinks.putAll(type, values);
        modifyLinks(true, EMPTYLINKS, removeLinks, null);
    }

    public void addLinks(String type, Set<String> values)
    {
        links.putAll(type, values);
        SetMultimap<String, String> addLinks = HashMultimap.create();
        addLinks.putAll(type, values);
        modifyLinks(true, addLinks, EMPTYLINKS, null);
    }

    public void setReadOnly(boolean readOnly)
    {
        this.readOnly = readOnly;
    }

    public void setRevision(int revision)
    {
        this.revision = revision;
    }

    public boolean isReadOnly()
    {
        return readOnly;
    }

    public String getOwnerName()
    {
        return ownerName;
    }

    public void setOwnerName(String ownerName)
    {
        this.ownerName = ownerName;
    }

    public String getCategory()
    {
        return category;
    }

    public void setCategory(String category)
    {
        this.category = category;
    }

    public Payload getPayload()
    {
        return payload;
    }

    public Buffer getBuffer()
    {
        return buffer;
    }

    public int getRevision()
    {
        return revision;
    }

    public AbstractIU(String uid)
    {
        this.uid = uid;
    }

    public boolean isCommitted()
    {
        return committed;
    }

    public void setBuffer(Buffer buffer)
    {
        this.buffer = buffer;
    }

    public boolean isPublished()
    {
        return buffer != null;
    }

    public String getUid()
    {
        return uid;
    }

    public void setUid(String uid)
    {
        this.uid = uid;
    }

    public abstract void commit();

    // XXX: might not be valid for all types of IUs
    public abstract void commit(String writerName);

    public void setPayload(Map<String, String> newPayload)
    {
        setPayload(newPayload, null);
    }

    public void setPayload(Map<String, String> newPayload, String writername)
    {
        List<PayloadItem> items = new ArrayList<PayloadItem>();
        for (Entry<String, String> entry : newPayload.entrySet())
        {
            PayloadItem item = PayloadItem.newBuilder().setKey(entry.getKey()).setValue(entry.getValue()).setType("") // TODO:default type?
                    .build();
            items.add(item);
        }
        setPayload(items, writername);
    }

    abstract void setPayload(List<PayloadItem> newItems, String writerName);

    abstract void putIntoPayload(String key, String value, String writer);

    abstract void removeFromPayload(Object key, String writer);

    abstract void handlePayloadSetting(List<PayloadItem> newPayload, String writerName);

    abstract void modifyLinks(boolean isDelta, SetMultimap<String, String> linksToAdd, SetMultimap<String, String> linksToRemove,
            String Writer);
}
