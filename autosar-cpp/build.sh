#!/bin/bash

# Build script for AUTOSAR C++ Learning Project
set -e

echo "=== Building AUTOSAR C++ Learning Project ==="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
make -j$(nproc)

echo "Build complete!"
echo ""
echo "To run the demo:"
echo "  ./autosar_demo"
echo ""
echo "To install (optional):"
echo "  sudo make install"