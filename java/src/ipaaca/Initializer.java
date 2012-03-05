package ipaaca;

import ipaaca.Ipaaca.IUCommission;
import ipaaca.Ipaaca.IULinkUpdate;
import ipaaca.Ipaaca.IUPayloadUpdate;
import rsb.converter.ConverterSignature;
import rsb.converter.DefaultConverterRepository;
import rsb.converter.ProtocolBufferConverter;

public final class Initializer {
//	def initialize_ipaaca_rsb():#{{{
//		rsb.transport.converter.registerGlobalConverter(
//			IntConverter(wireSchema="int32", dataType=int))
//		rsb.transport.converter.registerGlobalConverter(
//			IUConverter(wireSchema="ipaaca-iu", dataType=IU))
//		rsb.transport.converter.registerGlobalConverter(
//			IUPayloadUpdateConverter(
//				wireSchema="ipaaca-iu-payload-update",
//				dataType=IUPayloadUpdate))
//		rsb.transport.converter.registerGlobalConverter(
//			rsb.transport.converter.ProtocolBufferConverter(
//				messageClass=iuProtoBuf_pb2.IUCommission))
//		rsb.__defaultParticipantConfig = rsb.ParticipantConfig.fromDefaultSources()
//	#}}}
	public static void initializeIpaacaRsb()
	{
	    DefaultConverterRepository.getDefaultConverterRepository().addConverter(new IntConverter());
	    DefaultConverterRepository.getDefaultConverterRepository()
		  .addConverter(new ProtocolBufferConverter<IUCommission>(IUCommission.getDefaultInstance()));
	    
	    DefaultConverterRepository.getDefaultConverterRepository().addConverter(
	            new IUConverter(new ConverterSignature("ipaaca-iu", RemotePushIU.class)));
	    DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new IUConverter(new ConverterSignature("ipaaca-localiu", LocalIU.class)));
	    DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new PayloadConverter());
	    DefaultConverterRepository.getDefaultConverterRepository().addConverter(
                new LinkUpdateConverter());
	    	    
	}
}
