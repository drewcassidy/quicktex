
# Find dependencies
if (NOT QUICKTEX_NOPYTHON)
    find_package(Python COMPONENTS Interpreter Development.Module)
    find_package(pybind11 CONFIG REQUIRED)
endif ()
find_package(OpenMP)

#Collect source files
set(SOURCE_FILES
        Matrix4x4.cpp OldColor.cpp
        s3tc/bc1/BC1Block.cpp s3tc/bc1/BC1Decoder.cpp
        s3tc/bc1/BC1Encoder.cpp s3tc/bc1/OrderTable.cpp s3tc/bc1/OrderTable4.cpp
        s3tc/bc3/BC3Decoder.cpp s3tc/bc3/BC3Encoder.cpp
        s3tc/bc4/BC4Block.cpp s3tc/bc4/BC4Decoder.cpp s3tc/bc4/BC4Encoder.cpp
        s3tc/bc5/BC5Decoder.cpp s3tc/bc5/BC5Encoder.cpp
        s3tc/interpolator/Interpolator.cpp
        texture/RawTexture.cpp texture/Window.cpp test.cpp)

set(BINDING_FILES
        _bindings.cpp
        s3tc/_bindings.cpp
        s3tc/bc1/_bindings.cpp
        s3tc/bc3/_bindings.cpp
        s3tc/bc4/_bindings.cpp
        s3tc/bc5/_bindings.cpp
        s3tc/interpolator/_bindings.cpp)

file(GLOB_RECURSE HEADER_FILES "**.h")
file(GLOB_RECURSE PYTHON_FILES "**.py")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${BINDING_FILES} ${HEADER_FILES} ${PYTHON_FILES})

# Declare implementation module
add_library(quicktex STATIC ${SOURCE_FILES} ${HEADER_FILES})

# Link openMP if available
if (OpenMP_CXX_FOUND)
    target_link_libraries(quicktex PUBLIC OpenMP::OpenMP_CXX)
endif ()

# Link XSimd
target_link_libraries(quicktex PUBLIC xsimd)

# Set library features, like C/C++ standards
target_compile_features(quicktex PUBLIC cxx_std_20 c_std_11)
set_property(TARGET quicktex PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET quicktex PROPERTY POSITION_INDEPENDENT_CODE ON)

# Include source root for project-relative includes
target_include_directories(quicktex PUBLIC .)

# Set compiler warnings and SIMD flags
set_project_warnings(quicktex)
set_simd_flags(quicktex)

if (NOT QUICKTEX_NOPYTHON)
    # Declare python module
    pybind11_add_module(_quicktex ${BINDING_FILES} ${HEADER_FILES})
    target_compile_definitions(_quicktex PRIVATE VERSION_INFO=${QUICKTEX_VERSION_INFO})

    # Link python module with implementation
    target_link_libraries(_quicktex PUBLIC quicktex)
endif ()