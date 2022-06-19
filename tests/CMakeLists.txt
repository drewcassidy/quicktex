include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_HEADER_FILES "**.h")
file(GLOB_RECURSE TEST_SOURCE_FILES "**.cpp")
file(GLOB_RECURSE TEST_PYTHON_FILES "**.py")

# Organize source files together for some IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES} ${TEST_PYTHON_FILES})

add_executable(Test ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES})

target_link_libraries(Test PUBLIC quicktex gtest_main)

include(GoogleTest)
gtest_discover_tests(Test)

