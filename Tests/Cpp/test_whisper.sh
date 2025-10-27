#!/bin/bash

# Script to build and run Whisper integration tests
# Usage: ./test_whisper.sh

# Source common test setup and cleanup
source "$(dirname "$0")/test_setup.sh"

echo "=== Building and Running Whisper Integration Tests ==="

echo "Configuring build with CMake..."
# Copy the test CMakeLists to be the main one for this build
cp ../cmak_lists/test_whisper.cmake ./CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release .

echo "Building test executable..."
make

echo "Running whisper integration tests..."
echo "================================"
./bin/test_whisper

echo "================================"
echo "Test execution completed!"
