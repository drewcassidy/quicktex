cmake_minimum_required(VERSION 3.17)
project(python_rgbcx)

# Link to Pybind
add_subdirectory(extern/pybind11)

# Collect source files
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "src/*.h")
file(GLOB PYTHON_FILES "python/*.cpp" "python/*.h")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES} ${PYTHON_FILES})

# Add python module
pybind11_add_module(python_rgbcx
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES}
        )

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_17 c_std_11)