package ipaaca;

/**
 * IUReadOnlyException's occur when writing to a read-only IU
 * @author hvanwelbergen
 *
 */
public class IUReadOnlyException extends RuntimeException
{
    private static final long serialVersionUID = 1L;
    private final AbstractIU iu;

    public AbstractIU getIU()
    {
        return iu;
    }

    public IUReadOnlyException(AbstractIU iu)
    {
        super("Writing to IU " + iu.getUid() + " failed -- it is read-only.");
        this.iu = iu;
    }
}
