#!/bin/bash
# Build and run C++ tests for IArabicSpeech

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=== IArabicSpeech C++ Tests ==="
echo

# Check if CTranslate2 XCFramework is available
if [ ! -d "$SCRIPT_DIR/../../CTranslate2.xcframework" ]; then
    echo "⚠️  Warning: CTranslate2.xcframework not found"
    echo "   Expected at: $SCRIPT_DIR/../../CTranslate2.xcframework"
    echo "   The embedded XCFramework should be in the package root"
    echo
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring tests..."
cmake ..

# Build tests
echo
echo "Building tests..."
make -j$(sysctl -n hw.ncpu)

echo
echo "=== Running Tests ==="
echo

# Run each test
TESTS_PASSED=0
TESTS_FAILED=0

for test in feature_extractor_tests whisper_audio_tests whisper_tokenizer_tests; do
    if [ -f "./$test" ]; then
        echo
        echo "Running $test..."
        echo "----------------------------------------"
        if "./$test"; then
            TESTS_PASSED=$((TESTS_PASSED + 1))
            echo "✅ $test PASSED"
        else
            TESTS_FAILED=$((TESTS_FAILED + 1))
            echo "❌ $test FAILED"
        fi
        echo
    else
        echo "⚠️  Test executable not found: $test"
    fi
done

echo "=== Test Summary ==="
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"
echo

if [ $TESTS_FAILED -eq 0 ]; then
    echo "✅ All C++ tests passed!"
    exit 0
else
    echo "❌ Some C++ tests failed"
    exit 1
fi
