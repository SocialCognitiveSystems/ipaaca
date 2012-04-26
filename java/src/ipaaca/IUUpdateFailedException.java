package ipaaca;

/**
 * Indicates that a remote update failed
 * @author hvanwelbergen
 *
 */
public class IUUpdateFailedException extends RuntimeException
{
    private static final long serialVersionUID = 1L;
    private final AbstractIU iu;

    public AbstractIU getIU()
    {
        return iu;
    }

    public IUUpdateFailedException(AbstractIU iu)
    {
        super("Remote update failed for IU " + iu.getUid() + ".");
        this.iu = iu;
    }
}
