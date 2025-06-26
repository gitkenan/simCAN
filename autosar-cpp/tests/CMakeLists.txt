# Test configuration for AUTOSAR C++ Learning Project
cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # If Google Test is not found, download and build it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Test source files
set(TEST_SOURCES
    test_autosar_types.cpp
    test_swc_base.cpp
    test_engine_swc.cpp
    test_body_swc.cpp
    test_rte.cpp
    test_can_interface.cpp
    test_integration.cpp
)

# Create test executable
add_executable(autosar_tests ${TEST_SOURCES})

# Link against Google Test and our library
target_link_libraries(autosar_tests
    PRIVATE
    autosar_core
    gtest
    gtest_main
    Threads::Threads
)

# Add include directories
target_include_directories(autosar_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Discover tests for CTest
include(GoogleTest)
gtest_discover_tests(autosar_tests)

# Add custom target to run tests
add_custom_target(run_tests
    COMMAND autosar_tests
    DEPENDS autosar_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running AUTOSAR tests"
)