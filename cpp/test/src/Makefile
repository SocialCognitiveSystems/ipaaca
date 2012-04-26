CONFIG = -DIPAACA_DEBUG_MESSAGES
IPAACASOURCES = ../../src/ipaaca.cc ipaaca.pb.cc
TEXTSOURCES = ${IPAACASOURCES} textsender.cc
CCFLAGS=-I. -I../../src -I/usr/local/include -I/opt/local/include ${CONFIG}
BOOSTLIBS = -L/opt/local/lib -lboost_regex-mt -lboost_date_time-mt -lboost_program_options-mt -lboost_thread-mt -lboost_filesystem-mt -lboost_signals-mt -lboost_system-mt
PROTOLIBS = -L/opt/local/lib -lprotobuf
LIBS = ${BOOSTLIBS} ${PROTOLIBS} -L/usr/local/lib -lrsc -lrsbcore

COMPILER = gfilt

all: protoc textsender
	

textsender:
	${COMPILER} ${CCFLAGS} -o textsender ${TEXTSOURCES} ${LIBS}

protoc:
	protoc --proto_path=../../../proto ../../../proto/ipaaca.proto --cpp_out=.

clean:
	rm -f textsender ipaaca.pb.h ipaaca.pb.cc


