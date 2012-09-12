cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_cpp_example_component)

## use the following line to enable console debug messages in ipaaca
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPAACA_DEBUG_MESSAGES")

# find cmake modules locally too
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(Boost COMPONENTS system filesystem thread regex REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
link_directories(${PROTOBUF_LIBRARY_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

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

# add lib and include directory from pulled dependencies
include_directories( ${PROJECT_SOURCE_DIR}/../../../dist/include ${PROJECT_SOURCE_DIR}/../../../deps/include )
link_directories( ${PROJECT_SOURCE_DIR}/../../../dist/lib ${PROJECT_SOURCE_DIR}/../../../deps/lib )

# specify source files for ipaaca (auto-generated ones are in build/ )
set (SOURCE
	src/example-component.cc
	)

add_executable(example-component ${SOURCE})
target_link_libraries(example-component ${LIBS})

set(DEFAULT_BIN_SUBDIR bin)
set(DEFAULT_LIB_SUBDIR lib)
set(DEFAULT_DATA_SUBDIR share/data)
set(DEFAULT_INCLUDE_SUBDIR include)
set(CMAKE_INSTALL_PREFIX "")
install (
	TARGETS example-component
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

