cmake_minimum_required(VERSION 3.0)
project(DX12GS)

include(cmake/visualstudio.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(src)