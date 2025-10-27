#!/bin/bash

# Common test setup and cleanup for C++ tests
# Usage: source test_setup.sh

set -e  # Exit on any error

# Store the test directory path
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Cleanup function that runs on exit (success or failure)
cleanup() {
    echo "Cleaning up build directory..."
    cd "$TEST_DIR"
    if [ -d "build" ]; then
        rm -rf build
        echo "✅ build directory removed"
    fi
}

# Set trap to run cleanup on script exit
trap cleanup EXIT

# Navigate to script directory
cd "$TEST_DIR"

# Remove build directory if it exists from previous run
if [ -d "build" ]; then
    rm -rf build
fi

# Create and enter build directory
mkdir build && cd build
