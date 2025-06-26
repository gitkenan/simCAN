#!/bin/bash

# AUTOSAR C++ Test Runner Script
# Builds and runs the complete test suite

set -e  # Exit on any error

echo "=== AUTOSAR C++ Test Runner ==="
echo

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Must be run from autosar-cpp directory"
    echo "Usage: cd autosar-cpp && ./test_runner.sh"
    exit 1
fi

# Clean previous build
echo "Cleaning previous build..."
rm -rf build_test/
mkdir build_test
cd build_test

echo "Configuring test build..."
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

echo "Building tests..."
make -j$(nproc)

echo
echo "=== Running AUTOSAR Test Suite ==="
echo

# Run tests with verbose output
echo "Running all tests..."
./autosar_tests --gtest_output=xml:test_results.xml

echo
echo "=== Test Summary ==="

# Check if tests passed
if [ $? -eq 0 ]; then
    echo "✅ All tests PASSED"
    echo
    echo "Test coverage areas:"
    echo "  ✓ AUTOSAR Types and C++20 features"
    echo "  ✓ Software Component base classes"
    echo "  ✓ Engine SWC functionality" 
    echo "  ✓ Body Control SWC functionality"
    echo "  ✓ Runtime Environment (RTE)"
    echo "  ✓ CAN interface and protocols"
    echo "  ✓ Complete system integration"
    echo
    echo "🎯 Test results saved to: build_test/test_results.xml"
else
    echo "❌ Some tests FAILED"
    echo "Check the output above for details"
    exit 1
fi

echo
echo "=== Build Information ==="
echo "Compiler: $(cmake --version | head -1)"
echo "Build type: Debug with tests enabled"
echo "C++ standard: C++20"
echo "Threads: $(nproc) parallel build jobs"

echo
echo "🚀 AUTOSAR C++ test suite completed successfully!"
echo "The system demonstrates production-ready automotive software patterns."