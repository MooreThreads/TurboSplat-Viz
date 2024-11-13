cmake_minimum_required(VERSION 3.0)
project(DX12GS)

include(cmake/visualstudio.cmake)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)