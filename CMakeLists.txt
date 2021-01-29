cmake_minimum_required(VERSION 3.17)
project(python_rgbcx)

# Setup pybind
add_subdirectory(extern/pybind11)

# Make the pybind module
pybind11_add_module(python_rgbcx src/main.cpp)

# Set module features, like C/C++ standards
target_compile_features(python_rgbcx PUBLIC cxx_std_14 c_std_11)