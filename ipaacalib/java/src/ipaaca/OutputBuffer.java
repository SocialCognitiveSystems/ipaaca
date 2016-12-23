
/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".
 *
 * Copyright (c) 2009-2015 Social Cognitive Systems Group
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

import ipaaca.protobuf.Ipaaca;
import ipaaca.protobuf.Ipaaca.IUCommission;
import ipaaca.protobuf.Ipaaca.IURetraction;
import ipaaca.protobuf.Ipaaca.IUResendRequest;
import ipaaca.protobuf.Ipaaca.IULinkUpdate;
import ipaaca.protobuf.Ipaaca.IUPayloadUpdate;
import ipaaca.protobuf.Ipaaca.LinkSet;
import ipaaca.protobuf.Ipaaca.PayloadItem;

import java.util.HashMap;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import rsb.Factory;
import rsb.Informer;
import rsb.InitializeException;
import rsb.RSBException;
import rsb.patterns.DataCallback;
import rsb.patterns.EventCallback;
import rsb.patterns.LocalServer;
import rsb.Event;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.SetMultimap;

/**
 * An OutputBuffer that holds local IUs.
 * @author hvanwelbergen
 */
@Slf4j
public class OutputBuffer extends Buffer
{

    private final LocalServer server;
    private Map<String, Informer<Object>> informerStore = new HashMap<String, Informer<Object>>(); // category -> informer map
    private final static Logger logger = LoggerFactory.getLogger(OutputBuffer.class.getName());
    private IUStore<LocalIU> iuStore = new IUStore<LocalIU>();
    private String channel = "default";

    // def __init__(self, owning_component_name, participant_config=None):
    // '''Create an Output Buffer.
    //
    // Keyword arguments:
    // owning_component_name -- name of the entity that own this buffer
    // participant_config -- RSB configuration
    // '''
    // super(OutputBuffer, self).__init__(owning_component_name, participant_config)
    // self._unique_name = '/ipaaca/component/' + str(owning_component_name) + 'ID' + self._uuid + '/OB'
    // self._server = rsb.createServer(rsb.Scope(self._unique_name))
    // self._server.addMethod('updatePayload', self._remote_update_payload, IUPayloadUpdate, int)
    // self._server.addMethod('commit', self._remote_commit, iuProtoBuf_pb2.IUCommission, int)
    // self._informer_store = {}
    // self._id_prefix = str(owning_component_name)+'-'+str(self._uuid)+'-IU-'
    // self.__iu_id_counter_lock = threading.Lock()
    // self.__iu_id_counter = 0
    /**
     * @param owningComponentName name of the entity that own this buffer
     */
    public OutputBuffer(String owningComponentName)
    {
        this(owningComponentName, "default");

    }

    /**
     * @param owningComponentName name of the entity that own this buffer
     * @param channel name of the ipaaca channel this buffer is using
     */
    public OutputBuffer(String owningComponentName, String ipaaca_channel)
    {
        super(owningComponentName);

        uniqueName = "/ipaaca/component/" + getUniqueShortName() + "/OB";
        logger.debug("Creating server for {}", uniqueName);
        server = Factory.getInstance().createLocalServer(uniqueName);
        try
        {
            server.addMethod("updatePayload", new RemoteUpdatePayload());
            server.addMethod("updateLinks", new RemoteUpdateLinks());
            server.addMethod("commit", new RemoteCommit());
	    // add method to trigger a resend request. (dlw)
	    server.addMethod("resendRequest", new RemoteResendRequest());
            server.activate();
        }
        catch (InitializeException e)
        {
            throw new RuntimeException(e);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }

        this.channel = ipaaca_channel;
    }

    private final class RemoteUpdatePayload extends EventCallback //DataCallback<Integer, IUPayloadUpdate>
    {
        @Override
        public Event invoke(final Event request) //throws Throwable
        {
            logger.debug("remoteUpdate");
            int result = remoteUpdatePayload((IUPayloadUpdate) request.getData());
            //System.out.println("remoteUpdatePayload yielded revision "+result);
            return new Event(Integer.class, new Integer(result));
        }

    }
    /*private final class RemoteUpdatePayload extends DataCallback<Integer, IUPayloadUpdate>
    {
        @Override
        public Integer invoke(IUPayloadUpdate data) throws Throwable
        {
            logger.debug("remoteUpdate");
            return remoteUpdatePayload(data);
        }

    }*/

    private final class RemoteUpdateLinks extends EventCallback // DataCallback<Integer, IULinkUpdate>
    {
        @Override
        public Event invoke(final Event request) //throws Throwable
        {
            logger.debug("remoteUpdateLinks");
            return new Event(int.class, remoteUpdateLinks((IULinkUpdate) request.getData()));
        }

    }

    private final class RemoteCommit extends EventCallback //DataCallback<Integer, IUCommission>
    {
        @Override
        public Event invoke(final Event request) //throws Throwable
        {
            logger.debug("remoteCommit");
            return new Event(int.class, remoteCommit((IUCommission) request.getData()));
        }
    }

    private final class RemoteResendRequest extends EventCallback //DataCallback<Integer, IUResendRequest>
    {
        @Override
        public Event invoke(final Event request) //throws Throwable
        {
            logger.debug("remoteResendRequest");
            return new Event(int.class, remoteResendRequest((IUResendRequest) request.getData()));
        }
    }

    // def _remote_update_payload(self, update):
    // '''Apply a remotely requested update to one of the stored IUs.'''
    // if update.uid not in self._iu_store:
    // logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(update.uid))
    // return 0
    // iu = self._iu_store[update.uid]
    // if (update.revision != 0) and (update.revision != iu.revision):
    // # (0 means "do not pay attention to the revision number" -> "force update")
    // logger.warning("Remote write operation failed because request was out of date; IU "+str(update.uid))
    // return 0
    // if update.is_delta:
    // for k in update.keys_to_remove:
    // iu.payload.__delitem__(k, writer_name=update.writer_name)
    // for k,v in update.new_items.items():
    // iu.payload.__setitem__(k, v, writer_name=update.writer_name)
    // else:
    // iu._set_payload(update.new_items, writer_name=update.writer_name)
    // self.call_iu_event_handlers(update.uid, local=True, event_type=IUEventType.UPDATED, category=iu.category)
    // return iu.revision

    /**
     * Apply a remotely requested update to one of the stored IUs.
     * @return 0 if not updated, IU version number otherwise
     */
    int remoteUpdatePayload(IUPayloadUpdate update)
    {
        if (!iuStore.containsKey(update.getUid()))
        {
            logger.warn("Remote InBuffer tried to spuriously write non-existent IU {}", update.getUid());
            return 0;
        }

        AbstractIU iu = iuStore.get(update.getUid());
        if (update.getRevision() != 0 && update.getRevision() != iu.getRevision())
        {
            // (0 means "do not pay attention to the revision number" -> "force update")
            logger.warn("Remote write operation failed because request was out of date; IU {}", update.getUid());
            return 0;
        }
        if (update.getIsDelta())
        {
            for (String k : update.getKeysToRemoveList())
            {
                iu.getPayload().remove(k, update.getWriterName());
            }
            if (update.getNewItemsList().size() > 0)
            {
            	HashMap<String, String> payloadUpdate = new HashMap<String, String>();

            	for (PayloadItem pli : update.getNewItemsList())
            	{
            		payloadUpdate.put(pli.getKey(), pli.getValue());
            		//    //iu.getPayload().put(pli.getKey(), pli.getValue(), update.getWriterName());
            	}
            	iu.getPayload().putAll(payloadUpdate, update.getWriterName());
            }
        }
        else
        {
            iu.setPayload(update.getNewItemsList(), update.getWriterName());
        }
        callIuEventHandlers(update.getUid(), true, IUEventType.UPDATED, iu.getCategory());
        return iu.revision;
    }

    /**
     * Apply a remotely requested update to one of the stored IUs.
     * @return 0 if not updated, IU version number otherwise
     */
    int remoteUpdateLinks(IULinkUpdate update)
    {
        if (!iuStore.containsKey(update.getUid()))
        {
            logger.warn("Remote InBuffer tried to spuriously write non-existent IU {}", update.getUid());
            return 0;
        }

        AbstractIU iu = iuStore.get(update.getUid());
        if (update.getRevision() != 0 && update.getRevision() != iu.getRevision())
        {
            // (0 means "do not pay attention to the revision number" -> "force update")
            logger.warn("Remote write operation failed because request was out of date; IU {}", update.getUid());
            return 0;
        }
        if (update.getIsDelta())
        {
            SetMultimap<String, String> newLinks = HashMultimap.create();
            for (LinkSet ls : update.getNewLinksList())
            {
                newLinks.putAll(ls.getType(), ls.getTargetsList());
            }

            SetMultimap<String, String> removeLinks = HashMultimap.create();
            for (LinkSet ls : update.getLinksToRemoveList())
            {
                removeLinks.putAll(ls.getType(), ls.getTargetsList());
            }
            iu.modifyLinks(newLinks, removeLinks);
        }
        else
        {
            SetMultimap<String, String> newLinks = HashMultimap.create();
            for (LinkSet ls : update.getNewLinksList())
            {
                newLinks.putAll(ls.getType(), ls.getTargetsList());
            }
            iu.setLinks(newLinks);
        }
        callIuEventHandlers(update.getUid(), true, IUEventType.LINKSUPDATED, iu.getCategory());
        return iu.revision;
    }

    //
    // def _remote_commit(self, iu_commission):
    // '''Apply a remotely requested commit to one of the stored IUs.'''
    // if iu_commission.uid not in self._iu_store:
    // logger.warning("Remote InBuffer tried to spuriously write non-existent IU "+str(iu_commission.uid))
    // return 0
    // iu = self._iu_store[iu_commission.uid]
    // if (iu_commission.revision != 0) and (iu_commission.revision != iu.revision):
    // # (0 means "do not pay attention to the revision number" -> "force update")
    // logger.warning("Remote write operation failed because request was out of date; IU "+str(iu_commission.uid))
    // return 0
    // if iu.committed:
    // return 0
    // else:
    // iu._internal_commit(writer_name=iu_commission.writer_name)
    // self.call_iu_event_handlers(iu_commission.uid, local=True, event_type=IUEventType.COMMITTED, category=iu.category)
    // return iu.revision
    /**
     * Apply a remotely requested commit to one of the stored IUs.
     */
    private int remoteCommit(IUCommission iuc)
    {
        if (!iuStore.containsKey(iuc.getUid()))
        {
            logger.warn("Remote InBuffer tried to spuriously write non-existent IU {}", iuc.getUid());
            return 0;
        }
        AbstractIU iu = iuStore.get(iuc.getUid());
        if (iuc.getRevision() != 0 && iuc.getRevision() != iu.getRevision())
        {
            // (0 means "do not pay attention to the revision number" -> "force update")
            logger.warn("Remote write operation failed because request was out of date; IU {}", iuc.getUid());
            return 0;
        }
        if (iu.isCommitted())
        {
            return 0;
        }
        else
        {
            iu.commit(iuc.getWriterName());
            callIuEventHandlers(iuc.getUid(), true, IUEventType.COMMITTED, iu.getCategory());
            return iu.getRevision();
        }
    }

    /*
     * Resend an requested iu over the specific hidden channel. (dlw) TODO
     */
    private int remoteResendRequest(IUResendRequest iu_resend_request_pack)
    {
        if (!iuStore.containsKey(iu_resend_request_pack.getUid()))
        {
            logger.warn("Remote InBuffer tried to spuriously write non-existent IU {}", iu_resend_request_pack.getUid());
            return 0;
        }
        AbstractIU iu = iuStore.get(iu_resend_request_pack.getUid());
        if ((iu_resend_request_pack.hasHiddenScopeName() == true)&&(!iu_resend_request_pack.getHiddenScopeName().equals("")))
        {
	    Informer<Object> informer = getInformer(iu_resend_request_pack.getHiddenScopeName());
            try
            {
                informer.publish(iu);
            }
            catch (RSBException e)
            {
                throw new RuntimeException(e);
            }
            return iu.getRevision();
        } else
        {
            return 0;
        }
    }

    // def _get_informer(self, iu_category):
    // '''Return (or create, store and return) an informer object for IUs of the specified category.'''
    // if iu_category in self._informer_store:
    // return self._informer_store[iu_category]
    // informer_iu = rsb.createInformer(
    // rsb.Scope("/ipaaca/category/"+str(iu_category)),
    // config=self._participant_config,
    // dataType=object)
    // self._informer_store[iu_category] = informer_iu #new_tuple
    // logger.info("Added informer on "+iu_category)
    // return informer_iu #return new_tuple

    /**
     * Return (or create, store and return) an informer object for IUs of the specified category.
     */
    private Informer<Object> getInformer(String category)
    {
        if (informerStore.containsKey(category))
        {
            return informerStore.get(category);
        }
        Informer<Object> informer;
        try
        {
            informer = Factory.getInstance().createInformer("/ipaaca/channel/" + this.channel + "/category/" + category);
        }
        catch (InitializeException e1)
        {
            throw new RuntimeException(e1);
        }

        informerStore.put(category, informer);
        logger.info("Added informer on channel " + this.channel + " and category " + category);

        try
        {
            informer.activate();
        }
        catch (InitializeException e)
        {
            throw new RuntimeException(e);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
        return informer;
    }

    // def add(self, iu):
    // '''Add an IU to the IU store, assign an ID and publish it.'''
    // if iu._uid is not None:
    // raise IUPublishedError(iu)
    // iu.uid = self._generate_iu_uid()
    // self._iu_store[iu._uid] = iu
    // iu.buffer = self
    // self._publish_iu(iu)
    /**
     * Add an IU to the IU store, assign an ID and publish it.
     */
    public void add(LocalIU iu)
    {
        if (iuStore.get(iu.getUid()) != null)
        {
            throw new IUPublishedException(iu);
        }
        if(!(iu instanceof LocalMessageIU))
        {
            iuStore.put(iu.getUid(), iu);
        }
        iu.setBuffer(this);
        publishIU(iu);
    }

    // def _publish_iu(self, iu):
    // '''Publish an IU.'''
    // informer = self._get_informer(iu._category)
    // informer.publishData(iu)
    private void publishIU(AbstractIU iu)
    {
        Informer<Object> informer = getInformer(iu.getCategory());
        try
        {
            informer.publish(iu);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
    }

    // def _send_iu_commission(self, iu, writer_name):
    // '''Send IU commission.
    //
    // Keyword arguments:
    // iu -- the IU that has been committed to
    // writer_name -- name of the Buffer that initiated this commit, necessary
    // to enable remote components to filter out updates that originated
    // from their own operations
    // '''
    // # a raw Protobuf object for IUCommission is produced
    // # (unlike updates, where we have an intermediate class)
    // iu_commission = iuProtoBuf_pb2.IUCommission()
    // iu_commission.uid = iu.uid
    // iu_commission.revision = iu.revision
    // iu_commission.writer_name = iu.owner_name if writer_name is None else writer_name
    // # print('sending IU commission event')
    // informer = self._get_informer(iu._category)
    // informer.publishData(iu_commission)
    /**
     * @param iu the IU that has been committed to
     * @param writerName name of the Buffer that initiated this commit, necessary
     *            to enable remote components to filter out updates that originated
     *            from their own operations
     */
    protected void sendIUCommission(AbstractIU iu, String writerName)
    {
        IUCommission iuc = Ipaaca.IUCommission.newBuilder().setUid(iu.getUid()).setRevision(iu.getRevision())
                .setWriterName(writerName == null ? iu.getOwnerName() : writerName).build();
        Informer<Object> informer = getInformer(iu.getCategory());
        try
        {
            informer.publish(iuc);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
    }

    protected void sendIURetraction(AbstractIU iu)
    {
        IURetraction iuc = Ipaaca.IURetraction.newBuilder().setUid(iu.getUid()).setRevision(iu.getRevision()).build();
        Informer<Object> informer = getInformer(iu.getCategory());
        try
        {
            informer.publish(iuc);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
    }

    // def _send_iu_payload_update(self, iu, is_delta, revision, new_items=None, keys_to_remove=None, writer_name="undef"):
    // '''Send an IU payload update.
    //
    // Keyword arguments:
    // iu -- the IU being updated
    // is_delta -- whether the update concerns only a single payload item or
    // the whole payload dictionary
    // revision -- the new revision number
    // new_items -- a dictionary of new payload items
    // keys_to_remove -- a list of the keys that shall be removed from the
    // payload
    // writer_name -- name of the Buffer that initiated this update, necessary
    // to enable remote components to filter out updates that originate d
    // from their own operations
    // '''
    // if new_items is None:
    // new_items = {}
    // if keys_to_remove is None:
    // keys_to_remove = []
    // payload_update = IUPayloadUpdate(iu._uid, is_delta=is_delta, revision=revision)
    // payload_update.new_items = new_items
    // if is_delta:
    // payload_update.keys_to_remove = keys_to_remove
    // payload_update.writer_name = writer_name
    // informer = self._get_informer(iu._category)
    // informer.publishData(payload_update)

    protected void sendIUPayloadUpdate(AbstractIU iu, IUPayloadUpdate update)
    {
        Informer<Object> informer = getInformer(iu.getCategory());
        try
        {
            informer.publish(update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
    }

    protected void sendIULinkUpdate(AbstractIU iu, IULinkUpdate update)
    {
        Informer<Object> informer = getInformer(iu.getCategory());
        try
        {
            informer.publish(update);
        }
        catch (RSBException e)
        {
            throw new RuntimeException(e);
        }
    }

    @Override
    public AbstractIU getIU(String iuid)
    {
        return iuStore.get(iuid);
    }

    public void close()
    {
        try
        {
            server.deactivate();
        }
        catch (RSBException e)
        {
            log.warn("RSBException on deactivating server in close", e);
        }
        catch (InterruptedException e)
        {
            Thread.currentThread().interrupt();
        }
        for (Informer<?> informer : informerStore.values())
        {
            try
            {
                informer.deactivate();
            }
            catch (RSBException e)
            {
                log.warn("RSBException on deactivating informer {} in close", e, informer.toString());
            }
            catch (InterruptedException e)
            {
                Thread.currentThread().interrupt();
            }
        }
    }
}
