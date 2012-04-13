CONFIG = -DIPAACA_DEBUG_MESSAGES
IPAACASOURCES = ipaaca.cc ipaaca.pb.cc
SOURCES = ${IPAACASOURCES} ipaaca-test-main.cc
TEXTSOURCES = ${IPAACASOURCES} textsender.cc
CCFLAGS=-I. -I/usr/local/include -I/opt/local/include ${CONFIG}
BOOSTLIBS = -L/opt/local/lib -lboost_regex-mt -lboost_date_time-mt -lboost_program_options-mt -lboost_thread-mt -lboost_filesystem-mt -lboost_signals-mt -lboost_system-mt
PROTOLIBS = -L/opt/local/lib -lprotobuf
LIBS = ${BOOSTLIBS} ${PROTOLIBS} -L/usr/local/lib -lrsc -lrsbcore

COMPILER = gfilt

all: receiver sender
	

receiver:
	${COMPILER} ${CCFLAGS} -DMAKE_RECEIVER -o ipaaca-receiver ${SOURCES} ${LIBS}

sender:
	${COMPILER} ${CCFLAGS} -DMAKE_SENDER -o ipaaca-sender ${SOURCES} ${LIBS}

textsender:
	${COMPILER} ${CCFLAGS} -o textsender ${TEXTSOURCES} ${LIBS}

main:
	${COMPILER} ${CCFLAGS} -o ipaaca-main ${SOURCES} ${LIBS}

protoc:
	protoc --proto_path=../../proto ../../proto/ipaaca.proto --cpp_out=.

clean:
	rm -f ipaaca-main ipaaca-sender ipaaca-receiver textsender ipaaca.pb.h ipaaca.pb.cc


