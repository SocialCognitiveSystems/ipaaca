cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp_test)

# use C++11 (starting with proto v2 / ipaaca-c++ release 12)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## use the following line to enable console debug messages in ipaaca
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_DEBUG_MESSAGES")

# find cmake modules locally too
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(Boost COMPONENTS system filesystem thread regex unit_test_framework REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
link_directories(${PROTOBUF_LIBRARY_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

# for boost unit_test to create main()
add_definitions(-DBOOST_TEST_DYN_LINK)

#set(RSBLIBS rsc rsbcore)

set(LIBS ${LIBS} ipaaca )
set(LIBS ${LIBS} ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES})
#${RSBLIBS})

# enhance the default search paths (headers, libs ...)
set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}:/opt/local:${CMAKE_PREFIX_PATH})

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

# add local include directory
include_directories( ${PROJECT_SOURCE_DIR}/include )

# add includes and builds from parent dir (ipaaca lib)
include_directories( ${PROJECT_SOURCE_DIR}/../include )
include_directories( ${PROJECT_SOURCE_DIR}/../build )
link_directories( ${PROJECT_SOURCE_DIR}/../build )

# add lib and include directory from pulled dependencies
include_directories( ${PROJECT_SOURCE_DIR}/../../../deps/include )
link_directories( ${PROJECT_SOURCE_DIR}/../../../deps/lib )

# specify source files for ipaaca (auto-generated ones are in build/ )
set (SOURCE
	src/testipaaca.cc
	)

# compile all files to "ipaaca" shared library
add_executable(testipaaca ${SOURCE})
# and link all the required external libs (found above using find_package etc.)
target_link_libraries(testipaaca ${LIBS})

set(DEFAULT_BIN_SUBDIR bin)
set(DEFAULT_LIB_SUBDIR lib)
set(DEFAULT_DATA_SUBDIR share/data)
set(DEFAULT_INCLUDE_SUBDIR include)
set(CMAKE_INSTALL_PREFIX "")
install (
	TARGETS testipaaca
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)


enable_testing()
add_test(TestIpaacaCpp testipaaca)

