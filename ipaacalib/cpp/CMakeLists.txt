cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp)

set (CMAKE_CXX_STANDARD 11)

# use C++11 (starting with proto v2 / ipaaca-c++ release 12)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## use the following line to enable console debug messages in ipaaca
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_DEBUG_MESSAGES")

# expose the full RSB api in the headers (set only in ipaaca itself)
#  !! NOTE: at the moment required in any ipaaca cpp project in Windows !!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_EXPOSE_FULL_RSB_API")

## use the following line to enable building mock IUs (FakeIU)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_BUILD_MOCK_OBJECTS")

# find cmake modules locally too
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules )

if(WIN32) # Check if we are on Windows
	if(MSVC) # Check if we are using the Visual Studio compiler
		#set_target_properties(TestProject PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
		#
		#   Setup section for Windows build (using precompiled rsb + deps)
		#
		#   You need the rsx precompiled archive, even if you build rsb yourself,
		#   for the dependencies. Make sure to grab the right version (bitness
		#   and Visual Studio version). Tested with the rsx-0.10 branch.
		#   Please unpack the rsx archive into the repo dir (where ipaaca also is).
		#   Then set these environment variables before building rsb or ipaaca:
		#
		#   set BOOST_ROOT=%SOA_REPO_DIR%\rsx\boost
		#   set PROTOBUF_ROOT=%SOA_REPO_DIR%\rsx\protobuf
		#   set SPREAD_ROOT=%SOA_REPO_DIR%\rsx\spread
		#
		#
		
		#
		#  If you want to compile rsb locally, check out the soa project 'rsb', build it
		#  and use resolve.sh to pull its libraries into this project.
		#  On the other hand, if you simply want to use the precompiled rsb from inside
		#  "rsx" (it works but has no debug info), uncomment the following four lines.
		#
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsb-win-scs_64bit/include/rsc0.14 )
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsb-win-scs_64bit/include/rsb0.14 )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsb-win-scs_64bit/lib )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsb-win-scs_64bit/lib )
		
		set(RSBLIBS rsc0.14 rsb0.14)
		#set(RSBLIBS ${PROJECT_SOURCE_DIR}/../../../rsx/RSC-0.14.5-win32/rsc0.14.lib  ${PROJECT_SOURCE_DIR}/../../../rsx/RSB-0.14.2-win32/rsb0.14.lib )
		set(LIBS ${LIBS} rpcrt4)

                set(Boost_USE_STATIC_LIBS ON)
                set(Boost_USE_MULTITHREADED ON)
	
		find_package(Boost COMPONENTS date_time program_options system filesystem thread signals regex REQUIRED)
		link_directories(${Boost_LIBRARY_DIRS})
		include_directories(${Boost_INCLUDE_DIRS})
		
		# Windows linkage hack: overriding the determined libs to remove boost_thread (causes multiple-definition issues)
		set(CORRECT_BOOST_LIBS "")
		foreach(BLIB ${Boost_LIBRARIES})
			#message(STATUS "Boost lib: ${BLIB}")
			##message(STATUS "(Windows hack:) forcing vc110")
			##string(REPLACE "-vc140" "-vc110" BLIB_CONV ${BLIB})
			#string(REGEX MATCH "boost_thread[^/]+$" drop_item ${BLIB_CONV})
			#if(drop_item)
		#		message(STATUS "(Windows hack:) =====> Removing boost_thread library from the linkage list.")
		#	else(drop_item)
				list(APPEND CORRECT_BOOST_LIBS ${BLIB})
		#	endif(drop_item)
		endforeach(BLIB ${Boost_LIBRARIES})
		set(Boost_LIBRARIES ${CORRECT_BOOST_LIBS})
		
		# Using custom Protobuf script (from rsc) because it honors PROTOBUF_ROOT
		find_package(ProtocolBuffers REQUIRED)
		link_directories(${PROTOBUF_LIBRARY_DIRS})
		include_directories(${PROTOBUF_INCLUDE_DIRS})
		
		
	else()
		message(SEND_ERROR "Unsupported compiler! Please build with MSVC 14.0 (Visual Studio 2015).")
	endif()
else()
	#
	#
	# Setup section for Linux or OS X (using 'rsb' soa project)
	#
	#
	find_package(Boost COMPONENTS system filesystem thread regex signals REQUIRED)
	link_directories(${Boost_LIBRARY_DIRS})
	include_directories(${Boost_INCLUDE_DIRS})
	#set(BOOSTLIBS boost_regex-mt boost_date_time-mt boost_program_options-mt boost_thread-mt boost_filesystem-mt boost_signals-mt boost_system-mt)

	find_package(ProtocolBuffers REQUIRED)
	link_directories(${PROTOBUF_LIBRARY_DIRS})
	include_directories(${PROTOBUF_INCLUDE_DIRS})

	# change for each new rsb version
	if (DEFINED APPLE)
		set(RSBLIBS rsc0.14 rsb0.14)
		#set(RSBLIBS rsc0.10 rsb.0.10)
	else(DEFINED APPLE)
		set(RSBLIBS ${PROJECT_SOURCE_DIR}/../../deps/lib/librsc0.14.so ${PROJECT_SOURCE_DIR}/../../deps/lib/librsb0.14.so )
		set(LIBS ${LIBS} uuid)
	endif(DEFINED APPLE)
	# enhance the default search paths (headers, libs ...)
	set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}:/opt/local:${CMAKE_PREFIX_PATH})
	# MacPorts compatibility
	if (DEFINED APPLE)
		message(STATUS "Adding extra options for building on Mac OS X")
		set(CXX_DEFINES "${CXX_DEFINES} -D__MACOSX__")
		link_directories( /opt/local/lib )
		include_directories( /opt/local/include )
	endif(DEFINED APPLE)
endif(WIN32)


set(LIBS ${LIBS} ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES} ${RSBLIBS})

if(!WIN32)
set(LIBS ${LIBS} pthread)
endif(!WIN32)

# Hide the rsb-induced boost-signals warning (FOR NOW)
set(IPAACA_CXX_DEFINES "${IPAACA_CXX_DEFINES} -DBOOST_SIGNALS_NO_DEPRECATION_WARNING")

# Compiler defines copied from the old build system
set(IPAACA_CXX_DEFINES "${IPAACA_CXX_DEFINES} -D_BSD_SOURCE -DUSE_AV -DMGC_USE_DOUBLE -DLEDA_PREFIX -D__NO_CAST_TO_LOCAL_TYPE__ -DDBGLVL=0")

# Combine the extra compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_OLD_CODE_CONVENIENCE_FLAGS} ${IPAACA_CXX_DEFINES}")

# add for for each new rsb version
include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsc0.14 )
include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsb0.14 )
#include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsc0.10 )
#include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsb0.10 )

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
	src/ipaaca-buffers.cc
	src/ipaaca-internal.cc
	src/ipaaca-iuinterface.cc
	src/ipaaca-ius.cc
	src/ipaaca-links.cc
	src/ipaaca-locking.cc
	src/ipaaca-payload.cc
	src/ipaaca-cmdline-parser.cc
	src/ipaaca-string-utils.cc
	src/util/notifier.cc
	build/ipaaca/ipaaca.pb.cc
	)

set (JSON_TEST_SOURCE
	src/ipaaca.cc
	src/ipaaca-buffers.cc
	src/ipaaca-fake.cc
	src/ipaaca-internal.cc
	src/ipaaca-iuinterface.cc
	src/ipaaca-json.cc    # main
	src/ipaaca-locking.cc
	src/ipaaca-links.cc
	src/ipaaca-payload.cc
	src/ipaaca-cmdline-parser.cc
	src/ipaaca-string-utils.cc
	# more stuff going beyond the fake test case
	src/ipaaca-ius.cc
	build/ipaaca/ipaaca.pb.cc
	)

set (TESTER_SOURCE
	src/ipaaca-tester.cc    # main
	src/ipaaca.cc
	src/ipaaca-buffers.cc
	src/ipaaca-fake.cc
	src/ipaaca-internal.cc
	src/ipaaca-iuinterface.cc
	src/ipaaca-locking.cc
	src/ipaaca-links.cc
	src/ipaaca-payload.cc
	src/ipaaca-cmdline-parser.cc
	src/ipaaca-string-utils.cc
	# more stuff going beyond the fake test case
	src/ipaaca-ius.cc
	build/ipaaca/ipaaca.pb.cc
	)




# compile all files to "ipaaca" shared library
add_library(ipaaca SHARED ${SOURCE})
# and link all the required external libs (found above using find_package etc.)
target_link_libraries(ipaaca ${LIBS})

add_executable (ipaaca-test-json ${JSON_TEST_SOURCE})
target_link_libraries (ipaaca-test-json ${LIBS})

add_executable (ipaaca-tester-cpp ${TESTER_SOURCE})
target_link_libraries (ipaaca-tester-cpp ${LIBS})

set(DEFAULT_BIN_SUBDIR bin)
set(DEFAULT_LIB_SUBDIR lib)
set(DEFAULT_DATA_SUBDIR share/data)
set(DEFAULT_INCLUDE_SUBDIR include)
set(CMAKE_INSTALL_PREFIX "")
install (
	TARGETS ipaaca ipaaca-test-json ipaaca-tester-cpp
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)
install(
	DIRECTORY include
	DESTINATION .
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hh" PATTERN "*.hpp" PATTERN "*.inl"
	)
install(
	FILES build/ipaaca/ipaaca.pb.h
	DESTINATION include/ipaaca/
	)


