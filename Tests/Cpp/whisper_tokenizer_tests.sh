#!/bin/bash

# Script to build and run tokenizer unit tests
# Usage: ./whisper_tokenizer_tests.sh

# Source common test setup and cleanup
source "$(dirname "$0")/test_setup.sh"

echo "=== Building and Running Tokenizer Unit Tests ==="

echo "Configuring build with CMake..."
# Copy the test CMakeLists to be the main one for this build
cp ../cmak_lists/whisper_tokenizer_tests.cmak ./CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release .

echo "Building test executable..."
make

echo "Running tokenizer tests..."
echo "================================"
./test_whisper_tokenizer

echo "================================"
echo "Test execution completed!"

# Optional: Run with CTest for more detailed output
echo ""
echo "Running with CTest for detailed results..."
make test