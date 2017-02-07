cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp)

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

FIND_PACKAGE(RSC 0.16 REQUIRED)
FIND_PACKAGE(RSB 0.16 REQUIRED)

set(LIBS ${LIBS} ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES} ${RSB_LIBRARIES} ${RSC_LIBRARIES} pthread)

# Hide the rsb-induced boost-signals warning (FOR NOW)
set(IPAACA_CXX_DEFINES "${IPAACA_CXX_DEFINES} -DBOOST_SIGNALS_NO_DEPRECATION_WARNING")

# Compiler defines copied from the old build system
set(IPAACA_CXX_DEFINES "${IPAACA_CXX_DEFINES} -D_BSD_SOURCE -DUSE_AV -DMGC_USE_DOUBLE -DLEDA_PREFIX -D__NO_CAST_TO_LOCAL_TYPE__ -DDBGLVL=0")

# Combine the extra compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_OLD_CODE_CONVENIENCE_FLAGS} ${IPAACA_CXX_DEFINES}")

# add include dir for auto-generated headers placed in build/
include_directories( ${PROJECT_SOURCE_DIR}/build )

# add local include directory
include_directories( ${PROJECT_SOURCE_DIR}/include )

INCLUDE_DIRECTORIES(BEFORE SYSTEM
    ${RSC_INCLUDE_DIRS}
    ${RSB_INCLUDE_DIRS}
)
LINK_DIRECTORIES(
    ${RSC_LIBRARY_DIRS}
    ${RSB_LIBRARY_DIRS}
    ${UUID_LIBRARY_DIRS}
)

FIND_PACKAGE(PkgConfig REQUIRED)
pkg_search_module(UUID REQUIRED uuid)

set(LIBS ${LIBS} ${UUID_LIBRARIES})


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


# compile all files to "ipaaca" shared library
add_library(ipaaca SHARED ${SOURCE})
# and link all the required external libs (found above using find_package etc.)
target_link_libraries(ipaaca ${LIBS})

add_executable (ipaaca-test-json ${JSON_TEST_SOURCE})
target_link_libraries (ipaaca-test-json ${LIBS})

set(DEFAULT_BIN_SUBDIR bin)
set(DEFAULT_LIB_SUBDIR lib)
set(DEFAULT_DATA_SUBDIR share/data)
set(DEFAULT_INCLUDE_SUBDIR include)

install (
	TARGETS ipaaca
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

install (
	TARGETS ipaaca-test-json
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)

configure_package_config_file(ipaacaConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ipaacaConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/ipaaca
  PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
  )
  
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ipaacaConfig.cmake" DESTINATION lib/cmake/ipaaca)

