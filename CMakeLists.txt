cmake_minimum_required(VERSION 3.17)
project(python_rgbcx)

# Link to Pybind
add_subdirectory(extern/pybind11)

# Collect source files
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "src/*.h")
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

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_20 c_std_11)
target_compile_features(test_rgbcx PUBLIC cxx_std_20 c_std_11)

set_property(TARGET python_rgbcx test_rgbcx PROPERTY INTERPROCEDURAL_OPTIMIZATION True) #enable FLTO if available

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_property(TARGET python_rgbcx test_rgbcx PROPERTY OSX_ARCHITECTURES_RELEASE x86_64 arm64) #Mach-O fat binary for arm and x86
endif ()
