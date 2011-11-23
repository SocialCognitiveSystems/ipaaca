package ipaaca;

import ipaaca.Ipaaca.PayloadItem;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Skeleton implementation of Schlangen's incremental unit
 * @author hvanwelbergen
 *
 */
public abstract class AbstractIU {
	
	private Buffer buffer;
	protected String ownerName;	
	protected Payload payload;
	protected String category;
	protected boolean committed = false;
	private String uid;
	protected int revision;
	private boolean readOnly = false;
	
	public void setReadOnly(boolean readOnly)
    {
        this.readOnly = readOnly;
    }




    public void setRevision(int revision) {
		this.revision = revision;
	}


	
	
	public boolean isReadOnly() {
		return readOnly;
	}

	public String getOwnerName() {
		return ownerName;
	}

	public void setOwnerName(String ownerName) {
		this.ownerName = ownerName;
	}
	
	public String getCategory() {
		return category;
	}

	public void setCategory(String category) {
		this.category = category;
	}
	
	public Payload getPayload() {
		return payload;
	}

	public Buffer getBuffer() {
		return buffer;
	}

	
	public int getRevision() {
		return revision;
	}

	public AbstractIU(String uid)	
	{
		this.uid=uid;
	}
	
	public boolean isCommitted() {
		return committed;
	}

	public void setBuffer(Buffer buffer) {
		this.buffer = buffer;
	}

	public boolean isPublished()
	{
		return buffer!=null;
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
	
	//XXX: might not be valid for all types of IUs
	public abstract void commit(String writerName);
	
	public void setPayload(Map<String,String> newPayload)
	{
	    setPayload(newPayload,null);
	}
	
	public void setPayload(Map<String,String> newPayload, String writername)
	{
	    List<PayloadItem> items = new ArrayList<PayloadItem>();
	    for(Entry<String, String> entry: newPayload.entrySet())
	    {
	        PayloadItem item = PayloadItem.newBuilder()
	                .setKey(entry.getKey())
	                .setValue(entry.getValue())
	                .setType("")                   //TODO:default type?
	                .build();
	        items.add(item);
	    }
	    setPayload(items, writername);
	}
	
	abstract void setPayload(List<PayloadItem> newItems, String writerName);
	
	
	abstract void putIntoPayload(String key, String value, String writer);
	abstract void removeFromPayload(Object key, String writer);
	abstract void handlePayloadSetting(List<PayloadItem>newPayload, String writerName);
}
