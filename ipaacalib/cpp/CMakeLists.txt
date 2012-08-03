cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp)

## use the following line to enable console debug messages in ipaaca
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_DEBUG_MESSAGES")

# find cmake modules locally too
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(Boost COMPONENTS system filesystem thread regex REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
#set(BOOSTLIBS boost_regex-mt boost_date_time-mt boost_program_options-mt boost_thread-mt boost_filesystem-mt boost_signals-mt boost_system-mt)

find_package(Protobuf REQUIRED)
link_directories(${PROTOBUF_LIBRARY_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

#set(PROTOLIBS protobuf)
set(RSBLIBS rsc rsbcore)

#set(LIBS ${LIBS} ${BOOSTLIBS} ${PROTOLIBS} ${RSBLIBS})
#set(LIBS ${LIBS} ${PROTOLIBS} ${RSBLIBS})
set(LIBS ${LIBS} ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES} ${RSBLIBS})

if (NOT DEFINED APPLE)
	set(LIBS ${LIBS} uuid)
endif(NOT DEFINED APPLE)

# enhance the default search paths (headers, libs ...)
set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}:/opt/local:${CMAKE_PREFIX_PATH})

## Ace2 uses deprecated style (non-template friend functions and non-const char*s)
##  We just hide the warnings here
#set(CXX_OLD_CODE_CONVENIENCE_FLAGS "-Wno-non-template-friend -Wno-write-strings")

# Compiler defines copied from the old build system
set(CXX_DEFINES "-D_BSD_SOURCE -DUSE_AV -DMGC_USE_DOUBLE -DLEDA_PREFIX -D__NO_CAST_TO_LOCAL_TYPE__ -DDBGLVL=0")
if (DEFINED APPLE)
	message(STATUS "Adding extra options for building on Mac OS X")
	set(CXX_DEFINES "${CXX_DEFINES} -D__MACOSX__")
	link_directories( /opt/local/lib )
	include_directories( /opt/local/include )
endif(DEFINED APPLE)

# Combine the extra compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_OLD_CODE_CONVENIENCE_FLAGS} ${CXX_DEFINES}")

# add include dir for auto-generated headers placed in build/
include_directories( ${PROJECT_SOURCE_DIR}/build )

# add local include directory
include_directories( ${PROJECT_SOURCE_DIR}/include )
# add lib and include directory from pulled dependencies
include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include )
link_directories( ${PROJECT_SOURCE_DIR}/../../deps/lib )

# specify source files for ipaaca (auto-generated ones are in build/ )
set (SOURCE
	src/ipaaca.cc
	src/ipaaca-cmdline-parser.cc
	src/ipaaca-string-utils.cc
	build/ipaaca/ipaaca.pb.cc
	)

# compile all files to "ipaaca" shared library
add_library(ipaaca SHARED ${SOURCE})
# and link all the required external libs (found above using find_package etc.)
target_link_libraries(ipaaca ${LIBS})

set(DEFAULT_BIN_SUBDIR bin)
set(DEFAULT_LIB_SUBDIR lib)
set(DEFAULT_DATA_SUBDIR share/data)
set(DEFAULT_INCLUDE_SUBDIR include)
set(CMAKE_INSTALL_PREFIX "")
install (
	TARGETS ipaaca
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)
install(
	DIRECTORY include
	DESTINATION /
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hh" PATTERN "*.hpp" PATTERN "*.inl"
	)
install(
	FILES build/ipaaca/ipaaca.pb.h
	DESTINATION /include/ipaaca/
	)


