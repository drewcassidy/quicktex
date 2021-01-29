cmake_minimum_required(VERSION 3.17)
project(python_rgbcx)

# Setup pybind
add_subdirectory(extern/pybind11)

# Make the python_rgbcx module
add_library(python_rgbcx MODULE src/main.cpp src/rgbcx.cpp)

# Link to Pybind
target_link_libraries(python_rgbcx PRIVATE pybind11::module pybind11::lto)

pybind11_extension(python_rgbcx)
pybind11_strip(python_rgbcx)

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_14 c_std_11)