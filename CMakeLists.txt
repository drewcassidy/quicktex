cmake_minimum_required(VERSION 3.17)
include(CheckIPOSupported)
include(tools/CompilerWarnings.cmake)

project(rgbcx)

# Link to Pybind
find_package(Python COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(extern/pybind11)

# Collect source files
file(GLOB SOURCE_FILES "src/rgbcx/*.cpp" "src/rgbcx/BC*/*.cpp")
file(GLOB HEADER_FILES "src/rgbcx/*.h" "src/rgbcx/BC*/*.h")
file(GLOB PYTHON_FILES "src/rgbcx/bindings/*.cpp" "src/rgbcx/bindings/*.h")
file(GLOB TEST_FILES "tests/*.cpp")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES} ${PYTHON_FILES})

# Add python module
pybind11_add_module(_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES})

add_executable(test_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${TEST_FILES})

target_link_libraries(test_rgbcx PRIVATE pybind11::embed)

target_compile_definitions(test_rgbcx PRIVATE -DCUSTOM_SYS_PATH="${CMAKE_HOME_DIRECTORY}/env/lib/python3.8/site-packages")
message("\"${CMAKE_HOME_DIRECTORY}/env/lib/python3.8/site-packages\"")

# Set module features, like C/C++ standards
target_compile_features(_rgbcx PUBLIC cxx_std_20 c_std_11)
target_compile_features(test_rgbcx PUBLIC cxx_std_20 c_std_11)

set_project_warnings(_rgbcx)
set_project_warnings(test_rgbcx)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
endif ()
