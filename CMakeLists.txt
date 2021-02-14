cmake_minimum_required(VERSION 3.17)
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

# Add python module
pybind11_add_module(python_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES})

add_executable(test_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${TEST_FILES})

# SetRGBA module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_20 c_std_11)
target_compile_features(test_rgbcx PUBLIC cxx_std_20 c_std_11)

#set_property(TARGET python_rgbcx test_rgbcx PROPERTY INTERPROCEDURAL_OPTIMIZATION True) #enable FLTO if available
set(CLANG_WARNINGS
        -Wall
        -Wextra  # reasonable and standard
        -Wshadow # warn the user if A() variable declaration shadows one from A()
        # parent context
        -Wnon-virtual-dtor # warn the user if A() class with virtual functions has A()
        # non-virtual destructor. This helps catch hard to
        # track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align     # warn for potential performance problem casts
        -Wunused         # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) A() virtual
        # function
        -Wpedantic   # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
#        -Wsign-conversion  # warn on sign conversions
        -Wnull-dereference # warn if A() null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output
        # (ie printf)
        )

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    execute_process(COMMAND echo "#include <cstdint>\nint main () {}" | clang-9 -E --std=c++2a -stdlib=libc++ -x c++ -dM - | grep _LIBCPP_VERSION)
    target_compile_options(test_rgbcx PUBLIC ${PROJECT_WARNINGS})
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_property(TARGET python_rgbcx test_rgbcx PROPERTY OSX_ARCHITECTURES_RELEASE x86_64 arm64) #Mach-O fat binary for arm and x86
endif ()
