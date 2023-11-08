cmake_minimum_required(VERSION 3.0)

project(mymuduo)

aux_source_directory(. SRC_LIST)
include_directories(${CMAKE_SOURCE_DIR}/inclue/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS -g )

add_subdirectory(test)
add_subdirectory(demo)
