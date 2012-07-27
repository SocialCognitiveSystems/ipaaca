CONFIG = -DIPAACA_DEBUG_MESSAGES
#IPAACASOURCES = ../../src/ipaaca.cc ipaaca.pb.cc
#TEXTSOURCES = ${IPAACASOURCES} testipaaca.cc
TEXTSOURCES = testipaaca.cc
CCFLAGS=-I../../../../dist/include -I. -I../../src -I/usr/local/include -I/opt/local/include ${CONFIG}
BOOSTLIBS = -L/opt/local/lib -lboost_regex-mt -lboost_date_time-mt -lboost_program_options-mt -lboost_thread-mt -lboost_filesystem-mt -lboost_signals-mt -lboost_system-mt
PROTOLIBS = -L/opt/local/lib -lprotobuf
#LIBS = ${BOOSTLIBS} ${PROTOLIBS} -L/usr/local/lib -lrsc -lrsbcore
LIBS = -L../../../../dist/lib -lipaaca

COMPILER = gfilt

all: testipaaca
	

testipaaca:
	${COMPILER} ${CCFLAGS} -o testipaaca ${TEXTSOURCES} ${LIBS}

protoc:
	protoc --proto_path=../../../proto ../../../proto/ipaaca.proto --cpp_out=.

clean:
	rm -f testipaaca ipaaca.pb.h ipaaca.pb.cc


