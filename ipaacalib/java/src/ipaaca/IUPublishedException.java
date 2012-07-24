package ipaaca;

/**
 * IUPublishedException exceptions occur when publishing (=putting it in an output buffer) an already published IU.
 * @author hvanwelbergen
 * 
 */
public class IUPublishedException extends RuntimeException
{
    private static final long serialVersionUID = 1L;
    private final AbstractIU iu;

    public AbstractIU getIU()
    {
        return iu;
    }

    public IUPublishedException(AbstractIU iu)
    {
        super("IU " + iu.getUid() + " is already present in the output buffer.");
        this.iu = iu;
    }

}
