cmake_minimum_required(VERSION 3.17)
project(python_rgbcx)

# Make the python_rgbcx module
set(C_FILES
        src/main.cpp
        src/rgbcx.cpp
        src/tables.cpp
        )
add_library(python_rgbcx MODULE ${C_FILES})

# Link to Pybind
add_subdirectory(extern/pybind11)
target_link_libraries(python_rgbcx PRIVATE pybind11::module pybind11::lto)

pybind11_extension(python_rgbcx)
pybind11_strip(python_rgbcx)

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_14 c_std_11)