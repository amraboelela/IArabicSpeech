#!/bin/bash

# Script to build and run audio processing unit tests
# Usage: ./audio_tests.sh

set -e  # Exit on any error

# Cleanup function that runs on exit (success or failure)
cleanup() {
    echo "Cleaning up build directory..."
    if [ -d "build" ]; then
        rm -rf build
        echo "✅ build directory removed"
    fi
}

# Set trap to run cleanup on script exit
trap cleanup EXIT
echo "=== Building and Running Audio Processing Unit Tests ==="

# Navigate to cpp directory
cd "$(dirname "$0")"

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo "Configuring build with CMake..."
# Copy the test CMakeLists to be the main one for this build
cp ../cmak_lists/audio_tests.cmak ./CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release .

echo "Building test executable..."
make

echo "Running audio processing tests..."
echo "================================"
./test_audio

echo "================================"
echo "Test execution completed!"

# Optional: Run with CTest for more detailed output
echo ""
echo "Running with CTest for detailed results..."
make test

cd ..
rm -rf build

echo "Done!"
