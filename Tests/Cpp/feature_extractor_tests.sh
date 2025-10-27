#!/bin/bash

# Script to build and run feature extractor unit tests
# Usage: ./feature_extractor_tests.sh

# Source common test setup and cleanup
source "$(dirname "$0")/test_setup.sh"

echo "=== Building and Running Feature Extractor Unit Tests ==="

echo "Configuring build with CMake..."
# Copy the test CMakeLists to be the main one for this build
cp ../cmak_lists/feature_extractor_tests.cmak ./CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release .

echo "Building test executable..."
make

echo "Running feature extractor tests..."
echo "================================"
./test_feature_extractor

echo "================================"
echo "Test execution completed!"

# Optional: Run with CTest for more detailed output
echo ""
echo "Running with CTest for detailed results..."
make test