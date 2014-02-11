cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp)

## use the following line to enable console debug messages in ipaaca
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_DEBUG_MESSAGES")

# expose the full RSB api in the headers (set only in ipaaca itself)
#  !! NOTE: at the moment required in any ipaaca cpp project in Windows !!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_EXPOSE_FULL_RSB_API")

# find cmake modules locally too
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

if(WIN32) # Check if we are on Windows
	if(MSVC) # Check if we are using the Visual Studio compiler
		#set_target_properties(TestProject PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
		#
		#   Setup section for Windows build (using precompiled rsb + deps)
		#
		#   set BOOST_ROOT=%YOURREPODIR%\rsx\boost
		#   set PROTOBUF_INCLUDE_DIR=%YOURREPODIR%\rsx\protobuf\include
		#
		#
		#find_package(Boost COMPONENTS date_time program_options system filesystem thread signals regex REQUIRED)
		#link_directories(${Boost_LIBRARY_DIRS})
		#include_directories(${Boost_INCLUDE_DIRS})
		# overriding the determined libs to remove boost_thread (causes multiple-definition issues)
		
		set(Boost_LIBRARIES boost_regex-vc100-mt-gd-1_54 boost_date_time-vc100-mt-gd-1_54 boost_program_options-vc100-mt-gd-1_54 boost_filesystem-vc100-mt-gd-1_54 boost_signals-vc100-mt-gd-1_54 boost_system-vc100-mt-gd-1_54)
		
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/boost/include/boost-1_54 )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/boost/lib )
		
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/RSC-0.11.0-win32/include/rsc0.11 )
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/RSB-0.11.0-win32/include/rsb0.11 )
		include_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/protobuf/include )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/RSC-0.11.0-win32/lib )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/RSB-0.11.0-win32/lib )
		link_directories( ${PROJECT_SOURCE_DIR}/../../../rsx/protobuf/bin )
		
		set(RSBLIBS rsc0.11 rsb)
		set(PROTOBUF_LIBRARY libprotobuf)
		set(LIBS ${LIBS} rpcrt4)
		# boost_regex-vc100-mt-gd-1_54.lib;boost_date_time-vc100-mt-gd-1_54.lib;boost_program_options-vc100-mt-gd-1_54.lib;boost_filesystem-vc100-mt-gd-1_54.lib;boost_signals-vc100-mt-gd-1_54.lib;boost_system-vc100-mt-gd-1_54.lib
		
		#	boost_regex-mt boost_date_time-mt boost_program_options-mt boost_thread-mt boost_filesystem-mt boost_signals-mt boost_system-mt
	else()
		message(SEND_ERROR "Unsupported compiler! Please build with MSVC (2010).")
	endif()
else()
	#
	# Setup section for Linux or OS X (using 'rsb' soa project)
	#
	find_package(Boost COMPONENTS system filesystem thread regex REQUIRED)
	link_directories(${Boost_LIBRARY_DIRS})
	include_directories(${Boost_INCLUDE_DIRS})
	#set(BOOSTLIBS boost_regex-mt boost_date_time-mt boost_program_options-mt boost_thread-mt boost_filesystem-mt boost_signals-mt boost_system-mt)

	find_package(Protobuf REQUIRED)
	link_directories(${PROTOBUF_LIBRARY_DIRS})
	include_directories(${PROTOBUF_INCLUDE_DIRS})

	# add for for each new rsb version
	include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsc0.10 )
	include_directories( ${PROJECT_SOURCE_DIR}/../../deps/include/rsb0.10 )
	# change for each new rsb version
	if (DEFINED APPLE)
		set(RSBLIBS rsc0.10 rsb.0.10)
	else(DEFINED APPLE)
		set(RSBLIBS ${PROJECT_SOURCE_DIR}/../../deps/lib/librsc0.10.so ${PROJECT_SOURCE_DIR}/../../deps/lib/librsb.so.0.10 )
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

# Compiler defines copied from the old build system
set(CXX_DEFINES "-D_BSD_SOURCE -DUSE_AV -DMGC_USE_DOUBLE -DLEDA_PREFIX -D__NO_CAST_TO_LOCAL_TYPE__ -DDBGLVL=0")

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
	src/util/notifier.cc
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


