package ipaaca;

import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.util.Collection;
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
    private Map<String, String> map = new HashMap<String, String>();
    private final AbstractIU iu;

    public Payload(AbstractIU iu)
    {
        this.iu = iu;
    }

    // def __init__(self, remote_push_iu, new_payload):
    // """Create remote payload object.
    //
    // Keyword arguments:
    // remote_push_iu -- remote IU holding this payload
    // new_payload -- payload dict to initialise this remote payload with
    // """
    // super(RemotePushPayload, self).__init__()
    // self._remote_push_iu = remote_push_iu
    // if new_payload is not None:
    // for k,v in new_payload.items():
    // dict.__setitem__(self, k, v)
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
        map.clear();
        map.putAll(newPayload);
    }

    public void set(List<PayloadItem> newPayload, String writerName)
    {
        iu.handlePayloadSetting(newPayload, writerName);
        map.clear();
        for (PayloadItem item : newPayload)
        {
            map.put(item.getKey(), item.getValue());
        }
    }

    // def _remotely_enforced_setitem(self, k, v):
    // """Sets an item when requested remotely."""
    // return dict.__setitem__(self, k, v)
    void enforcedSetItem(String key, String value)
    {
        map.put(key, value);
    }

    // def _remotely_enforced_delitem(self, k):
    // """Deletes an item when requested remotely."""
    // return dict.__delitem__(self, k)
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

    public Set<java.util.Map.Entry<String, String>> entrySet()
    {
        return map.entrySet();
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

    // def __setitem__(self, k, v):
    // """Set item in this payload.
    //
    // Requests item setting from the OutputBuffer holding the local version
    // of this IU. Returns when permission is granted and item is set;
    // otherwise raises an IUUpdateFailedError.
    // """
    // if self._remote_push_iu.committed:
    // raise IUCommittedError(self._remote_push_iu)
    // if self._remote_push_iu.read_only:
    // raise IUReadOnlyError(self._remote_push_iu)
    // requested_update = IUPayloadUpdate(
    // uid=self._remote_push_iu.uid,
    // revision=self._remote_push_iu.revision,
    // is_delta=True,
    // writer_name=self._remote_push_iu.buffer.unique_name,
    // new_items={k:v},
    // keys_to_remove=[])
    // remote_server = self._remote_push_iu.buffer._get_remote_server(self._remote_push_iu)
    // new_revision = remote_server.updatePayload(requested_update)
    // if new_revision == 0:
    // raise IUUpdateFailedError(self._remote_push_iu)
    // else:
    // self._remote_push_iu._revision = new_revision
    // dict.__setitem__(self, k, v)
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

    //
    // def __delitem__(self, k):
    // """Delete item in this payload.
    //
    // Requests item deletion from the OutputBuffer holding the local version
    // of this IU. Returns when permission is granted and item is deleted;
    // otherwise raises an IUUpdateFailedError.
    // """
    // if self._remote_push_iu.committed:
    // raise IUCommittedError(self._remote_push_iu)
    // if self._remote_push_iu.read_only:
    // raise IUReadOnlyError(self._remote_push_iu)
    // requested_update = IUPayloadUpdate(
    // uid=self._remote_push_iu.uid,
    // revision=self._remote_push_iu.revision,
    // is_delta=True,
    // writer_name=self._remote_push_iu.buffer.unique_name,
    // new_items={},
    // keys_to_remove=[k])
    // remote_server = self._remote_push_iu.buffer._get_remote_server(self._remote_push_iu)
    // new_revision = remote_server.updatePayload(requested_update)
    // if new_revision == 0:
    // raise IUUpdateFailedError(self._remote_push_iu)
    // else:
    // self._remote_push_iu._revision = new_revision
    // dict.__delitem__(self, k)
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

    public void putAll(Map<? extends String, ? extends String> m)
    {
        throw new RuntimeException("Not implemented");
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
