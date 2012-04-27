package ipaaca;

/**
 * Error indicating that an IU is immutable because it has been committed to.
 * @author hvanwelbergen
 * 
 */
public class IUCommittedException extends RuntimeException
{
    private static final long serialVersionUID = 1L;
    private final AbstractIU iu;

    public AbstractIU getIU()
    {
        return iu;
    }

    public IUCommittedException(AbstractIU iu)
    {
        super("Writing to IU " + iu.getUid() + " failed -- it has been committed to.");
        this.iu = iu;
    }
}
