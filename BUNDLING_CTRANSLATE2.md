# CTranslate2 Binary Bundling Guide

This document explains how to bundle the CTranslate2 shared library with IArabicSpeech.

## Overview

Instead of requiring users to install CTranslate2 system-wide, IArabicSpeech now expects the CTranslate2 binary to be bundled within the package.

## Directory Structure

Create the following directory structure in your package:

```
Sources/IArabicSpeech/
├── ctranslate2/
│   ├── include/          # CTranslate2 header files
│   │   └── ctranslate2/
│   │       ├── models/
│   │       │   └── whisper.h
│   │       ├── storage_view.h
│   │       ├── vocabulary.h
│   │       └── ... (other headers)
│   └── lib/              # CTranslate2 shared libraries
│       ├── libctranslate2.dylib  # macOS (x86_64 and arm64 universal binary)
│       └── libctranslate2.so     # Linux (optional)
├── whisper/
├── include/
└── ... (other source files)
```

## Step 1: Build or Obtain CTranslate2 Binaries

### Option A: Build from Source (Recommended)

```bash
# Clone CTranslate2
git clone https://github.com/OpenNMT/CTranslate2.git
cd CTranslate2

# For macOS arm64 (Apple Silicon)
mkdir build-arm64
cd build-arm64
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=./install \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DWITH_OPENBLAS=ON \
  -DWITH_MKL=OFF \
  -DOPENMP_RUNTIME=NONE \
  -DBUILD_CLI=OFF

make -j8
make install

# For macOS x86_64 (Intel)
cd ..
mkdir build-x86_64
cd build-x86_64
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=./install \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DWITH_OPENBLAS=ON \
  -DWITH_MKL=OFF \
  -DOPENMP_RUNTIME=NONE \
  -DBUILD_CLI=OFF

make -j8
make install

# Create universal binary (macOS only)
lipo -create \
  build-arm64/install/lib/libctranslate2.dylib \
  build-x86_64/install/lib/libctranslate2.dylib \
  -output libctranslate2.dylib

# Verify architectures
lipo -info libctranslate2.dylib
# Expected output: Architectures in the fat file: libctranslate2.dylib are: x86_64 arm64
```

### Option B: Download Pre-built Binaries

If available from the CTranslate2 releases page:
1. Download the appropriate release for your platform
2. Extract the shared library files

## Step 2: Copy Files to Package

```bash
# From your IArabicSpeech package root
cd /path/to/IArabicSpeech

# Create directory structure
mkdir -p Sources/IArabicSpeech/ctranslate2/include
mkdir -p Sources/IArabicSpeech/ctranslate2/lib

# Copy headers (from CTranslate2 build)
cp -R /path/to/CTranslate2/build-arm64/install/include/ctranslate2 \
  Sources/IArabicSpeech/ctranslate2/include/

# Copy universal binary
cp libctranslate2.dylib Sources/IArabicSpeech/ctranslate2/lib/

# Set proper permissions
chmod 755 Sources/IArabicSpeech/ctranslate2/lib/libctranslate2.dylib
```

## Step 3: Update Library ID and RPATH (macOS)

```bash
cd Sources/IArabicSpeech/ctranslate2/lib

# Update the library's install name
install_name_tool -id "@rpath/libctranslate2.dylib" libctranslate2.dylib

# Check dependencies
otool -L libctranslate2.dylib

# If CTranslate2 depends on other libraries (e.g., OpenBLAS), you may need to update their paths:
# install_name_tool -change /usr/local/lib/libopenblas.dylib @rpath/libopenblas.dylib libctranslate2.dylib
```

## Step 4: Verify Package Structure

```bash
# Check that files are in place
ls -la Sources/IArabicSpeech/ctranslate2/include/ctranslate2/
ls -la Sources/IArabicSpeech/ctranslate2/lib/

# Verify binary
file Sources/IArabicSpeech/ctranslate2/lib/libctranslate2.dylib
lipo -info Sources/IArabicSpeech/ctranslate2/lib/libctranslate2.dylib
```

## Step 5: Build and Test

```bash
# Build the package
swift build

# Run tests
swift test
```

## Troubleshooting

### Error: library not found for -lctranslate2

**Solution**: Ensure `libctranslate2.dylib` is in `Sources/IArabicSpeech/ctranslate2/lib/`

### Error: dyld: Library not loaded

**Solution**: Check the library's install name and RPATH:

```bash
otool -L .build/debug/libIArabicSpeech.dylib
install_name_tool -id "@rpath/libctranslate2.dylib" Sources/IArabicSpeech/ctranslate2/lib/libctranslate2.dylib
```

### Error: Missing CTranslate2 headers

**Solution**: Verify headers are in correct location:

```bash
ls Sources/IArabicSpeech/ctranslate2/include/ctranslate2/models/whisper.h
ls Sources/IArabicSpeech/ctranslate2/include/ctranslate2/storage_view.h
```

## Dependencies

CTranslate2 may have runtime dependencies:

- **OpenBLAS** or **MKL**: For optimized linear algebra
- **zlib**: For compression (usually available system-wide)

### Bundling Dependencies (Optional)

If you want to bundle all dependencies:

```bash
# Copy OpenBLAS (if used)
cp /usr/local/lib/libopenblas.dylib Sources/IArabicSpeech/ctranslate2/lib/

# Update references
install_name_tool -change \
  /usr/local/lib/libopenblas.dylib \
  @rpath/libopenblas.dylib \
  Sources/IArabicSpeech/ctranslate2/lib/libctranslate2.dylib
```

## Size Considerations

- **libctranslate2.dylib**: ~10-20 MB (universal binary)
- **Headers**: ~1-2 MB

Total package size increase: ~11-22 MB

## Distribution

When distributing your package:

1. ✅ Include `Sources/IArabicSpeech/ctranslate2/` directory
2. ✅ Ensure binary has correct permissions (755)
3. ✅ Test on both Intel and Apple Silicon Macs
4. ✅ Document any additional dependencies users may need

## Alternative: XCFramework (iOS Support)

For iOS support, create an XCFramework:

```bash
# Build for iOS arm64
cmake .. -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0

# Build for iOS Simulator
cmake .. -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DCMAKE_OSX_SYSROOT=iphonesimulator

# Create XCFramework
xcodebuild -create-xcframework \
  -library build-ios/libctranslate2.a \
  -library build-simulator/libctranslate2.a \
  -output CTranslate2.xcframework
```

## Summary

Package.swift is now configured to:
- Look for CTranslate2 headers in `Sources/IArabicSpeech/ctranslate2/include/`
- Link against `libctranslate2.dylib` in `Sources/IArabicSpeech/ctranslate2/lib/`
- Set RPATH to find the bundled library at runtime

NO_CTRANSLATE2 flag has been **removed** - CTranslate2 is now always required.
