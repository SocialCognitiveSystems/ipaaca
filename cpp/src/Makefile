CONFIG = -DIPAACA_DEBUG_MESSAGES
SOURCES = ipaaca.cc ipaaca.pb.cc ipaaca-test-main.cc
CCFLAGS=-I. -I/usr/local/include -I/opt/local/include ${CONFIG}
BOOSTLIBS = -L/opt/local/lib -lboost_regex-mt -lboost_date_time-mt -lboost_program_options-mt -lboost_thread-mt -lboost_filesystem-mt -lboost_signals-mt -lboost_system-mt
PROTOLIBS = -L/opt/local/lib -lprotobuf
LIBS = ${BOOSTLIBS} ${PROTOLIBS} -L/usr/local/lib -lrsc -lrsbcore
all: protoc
	#g++ ${CCFLAGS} -o ipaaca-test ${SOURCES} ${LIBS}
	g++ ${CCFLAGS} -DMAKE_RECEIVER -o ipaaca-receiver ${SOURCES} ${LIBS}
	g++ ${CCFLAGS} -DMAKE_SENDER -o ipaaca-sender ${SOURCES} ${LIBS}

protoc:
	protoc --proto_path=../../proto ../../proto/ipaaca.proto --cpp_out=.

clean:
	rm -f ipaaca-test ipaaca-sender ipaaca-receiver ipaaca.pb.h ipaaca.pb.cc


