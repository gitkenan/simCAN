# CMakeLists.txt for AUTOSAR C++ Learning Project
cmake_minimum_required(VERSION 3.16)

# Project configuration
project(autosar_learning
    VERSION 1.0.0
    DESCRIPTION "AUTOSAR C++ Learning Implementation"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall                   # Enable most warning messages
        -Wextra                 # Enable extra warning messages
        -Wpedantic              # Issue warnings for pedantic ISO C violations
        -Werror                 # Treat warnings as errors
        -Wno-unused-parameter   # Don't warn about unused parameters (common in automotive)
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(
        /W4                     # Enable level 4 warnings
        /WX                     # Treat warnings as errors
    )
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(AUTOSAR_SOURCES
    src/swc_base.cpp
    src/can_interface.cpp
    src/engine_swc.cpp
    src/body_swc.cpp
    src/rte.cpp
)

set(AUTOSAR_HEADERS
    include/autosar_types.hpp
    include/swc_base.hpp
    include/can_interface.hpp
    include/engine_swc.hpp
    include/body_swc.hpp
    include/rte.hpp
)

# Main library
add_library(autosar_core STATIC ${AUTOSAR_SOURCES} ${AUTOSAR_HEADERS})

# Set library properties
set_target_properties(autosar_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${AUTOSAR_HEADERS}"
)

# Find required packages
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(autosar_core PRIVATE Threads::Threads)

# Example application (will be created next)
add_executable(autosar_demo
    examples/main_demo.cpp
)

target_link_libraries(autosar_demo PRIVATE autosar_core Threads::Threads)

# Testing (optional - can be enabled later)
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation (optional)
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# Installation
install(TARGETS autosar_core autosar_demo
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/autosar
)

# Create config file for other projects to use this library
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/autosar_learning-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/autosar_learning-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/autosar_learning-config.cmake"
    INSTALL_DESTINATION lib/cmake/autosar_learning
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/autosar_learning-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/autosar_learning-config-version.cmake"
    DESTINATION lib/cmake/autosar_learning
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build docs: ${BUILD_DOCS}")
message(STATUS "")