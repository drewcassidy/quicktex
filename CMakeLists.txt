cmake_minimum_required(VERSION 3.18)
include(tools/CompilerWarnings.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(quicktex)

# Find dependencies
find_package(Python COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenMP)

# Collect source files
file(GLOB SOURCE_FILES
        "quicktex/*.cpp"
        "quicktex/s3tc/*.cpp"
        "quicktex/s3tc/bc1/*.cpp"
        "quicktex/s3tc/bc3/*.cpp"
        "quicktex/s3tc/bc4/*.cpp"
        "quicktex/s3tc/bc5/*.cpp"
        "quicktex/s3tc/interpolator/*.cpp"
        )

file(GLOB HEADER_FILES
        "quicktex/*.h"
        "quicktex/s3tc/*.h"
        "quicktex/s3tc/bc1/*.h"
        "quicktex/s3tc/bc3/*.h"
        "quicktex/s3tc/bc4/*.h"
        "quicktex/s3tc/bc5/*.h"
        "quicktex/s3tc/interpolator/*.h"
        )

file(GLOB_RECURSE PYTHON_FILES "src/**/*.py")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES} ${PYTHON_FILES})

# Add python module
pybind11_add_module(_quicktex
        ${SOURCE_FILES}
        ${HEADER_FILES})

# Set Quicktex version info
target_compile_definitions(_quicktex PRIVATE VERSION_INFO=${QUICKTEX_VERSION_INFO})

# enable openMP if available
if (OpenMP_CXX_FOUND)
    target_link_libraries(_quicktex PUBLIC OpenMP::OpenMP_CXX)
endif ()

# Set module features, like C/C++ standards
target_compile_features(_quicktex PUBLIC cxx_std_17 c_std_11)

# Set compiler warnings
set_project_warnings(_quicktex)

# Clang-specific
if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++ -fsanitize=undefined")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
endif ()
