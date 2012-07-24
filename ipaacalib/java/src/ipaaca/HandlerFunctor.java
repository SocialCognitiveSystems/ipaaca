package ipaaca;

public interface HandlerFunctor
{
    void handle(AbstractIU iu, IUEventType type, boolean local);
}
