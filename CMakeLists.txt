cmake_minimum_required(VERSION 2.6)

project(Summarization)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/lib/ilclient ${SDKSTAGE}/opt/vc/include/ ${SDKSTAGE}/opt/vc/include/interface/vcos/pthread    s ${SDKSTAGE}/opt/vc/include/interface/vmcs_host/linux)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(lib)
add_subdirectory(src)
