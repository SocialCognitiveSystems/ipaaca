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
    private Initializer(){}
    public static void initializeIpaacaRsb()
    {
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new IntConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new ProtocolBufferConverter<IUCommission>(IUCommission.getDefaultInstance()));

        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-iu", RemotePushIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-localiu", LocalIU.class)));
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new PayloadConverter());
        DefaultConverterRepository.getDefaultConverterRepository().addConverter(new LinkUpdateConverter());

    }
}
