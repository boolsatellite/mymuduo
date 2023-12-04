cmake_minimum_required(VERSION 3.0)

project(mymuduo)

include_directories(${CMAKE_SOURCE_DIR}/include/)
aux_source_directory(. SRC_LIST)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS -g )

add_library(mymuduo ${SRC_LIST} include/EventLoopThread.h)

add_subdirectory(test)
#add_subdirectory(demo)
