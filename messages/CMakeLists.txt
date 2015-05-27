cmake_minimum_required (VERSION 2.8.3)

#Get c++11
ADD_DEFINITIONS(-fPIC)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../lib/")

file(GLOB ProtoFiles *.proto)

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles}) 
add_library(pylonGPSMessages STATIC ${ProtoSources} ${ProtoHeaders})
target_link_libraries(pylonGPSMessages ${PROTOBUF_LIBRARY})
