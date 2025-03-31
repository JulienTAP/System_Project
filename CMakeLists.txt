cmake_minimum_required(VERSION 3.28)

project(ShellProgram C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/arch)


set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDES_DIR ${PROJECT_ROOT}/include)

add_subdirectory(libs)
add_subdirectory(src)