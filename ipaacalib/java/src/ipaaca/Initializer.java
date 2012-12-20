package ipaaca;

import ipaaca.protobuf.Ipaaca.IUCommission;
import rsb.converter.ConverterSignature;
import rsb.converter.DefaultConverterRepository;
import rsb.converter.ProtocolBufferConverter;

/**
 * Hooks up the ipaaca converters, call initializeIpaacaRsb() before using ipaaca.
 * @author hvanwelbergen
 * 
 */
public final class Initializer
{
    private Initializer()
    {
    }
    private static volatile boolean initialized = false;
    
    public synchronized static void initializeIpaacaRsb()
    {
        if(initialized)return;
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new IntConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new ProtocolBufferConverter<IUCommission>(IUCommission.getDefaultInstance()));

        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-iu", RemotePushIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
         new IUConverter(new ConverterSignature("ipaaca-localiu", LocalIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-messageiu", RemoteMessageIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-localmessageiu", LocalMessageIU.class)));
         
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new PayloadConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new LinkUpdateConverter());
        initialized = true;
    }
}
