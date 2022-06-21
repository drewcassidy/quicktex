cmake_minimum_required(VERSION 3.18)
include(tools/CompilerWarnings.cmake)
include(tools/SIMDFlags.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(quicktex)

add_subdirectory(external/xsimd)

add_subdirectory(quicktex)
add_subdirectory(tests)

enable_testing ()
add_test (NAME QuicktexTest COMMAND Test)