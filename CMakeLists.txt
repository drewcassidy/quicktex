cmake_minimum_required(VERSION 3.17)
include(CheckIPOSupported)
include(tools/CompilerWarnings.cmake)

project(python_rgbcx)

# Link to Pybind
add_subdirectory(extern/pybind11)

# Collect source files
file(GLOB SOURCE_FILES "src/*.cpp" "src/BC*/*.cpp")
file(GLOB HEADER_FILES "src/*.h" "src/BC*/*.h")
file(GLOB PYTHON_FILES "python/*.cpp" "python/*.h")
file(GLOB TEST_FILES "src/test/*.c" "src/test/*.cpp" "src/test/*.h")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES} ${PYTHON_FILES})

include_directories(${CMAKE_SOURCE_DIR}/extern/gif-h)

# Add python module
pybind11_add_module(python_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES})

add_executable(test_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${TEST_FILES})

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_20 c_std_11)
target_compile_features(test_rgbcx PUBLIC cxx_std_20 c_std_11)

set_project_warnings(python_rgbcx)
set_project_warnings(test_rgbcx)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
endif ()
