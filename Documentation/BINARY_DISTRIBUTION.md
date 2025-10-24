# CTranslate2 Binary Distribution

This document explains the recommended approach for distributing CTranslate2 with IArabicSpeech.

## Recommended Approach: Binary Package

Instead of bundling the CTranslate2 binary directly in the source package, we recommend creating a **separate binary package** using Swift Package Manager's `binaryTarget` feature.

### Advantages

1. **Smaller Source Package**: Source code remains lightweight
2. **Platform-Specific Binaries**: Different binaries for macOS/iOS/Linux
3. **Version Management**: Separate versioning for binaries
4. **Faster Downloads**: Users only download binaries for their platform
5. **Easier Updates**: Update binaries without changing source code

## Implementation Options

### Option 1: XCFramework (Recommended for Apple Platforms)

Create an XCFramework containing CTranslate2 for all Apple platforms:

**Package.swift**:
```swift
let package = Package(
    name: "IArabicSpeech",
    products: [
        .library(name: "IArabicSpeech", targets: ["IArabicSpeech"]),
    ],
    targets: [
        .target(
            name: "IArabicSpeech",
            dependencies: ["CTranslate2"],
            cxxSettings: [
                .headerSearchPath("whisper"),
                .headerSearchPath("include"),
            ]
        ),
        .binaryTarget(
            name: "CTranslate2",
            url: "https://github.com/YOUR_USERNAME/CTrans late2-xcframework/releases/download/v3.20.0/CTranslate2.xcframework.zip",
            checksum: "..." // SHA256 checksum
        ),
        .testTarget(
            name: "IArabicSpeechTests",
            dependencies: ["IArabicSpeech"]
        ),
    ]
)
```

### Option 2: Local Binary (Development)

For local development, use a local binary target:

**Package.swift**:
```swift
.binaryTarget(
    name: "CTranslate2",
    path: "Binaries/CTranslate2.xcframework"
)
```

**Directory Structure**:
```
IArabicSpeech/
├── Sources/
├── Tests/
├── Binaries/
│   └── CTranslate2.xcframework/
│       ├── macos-arm64/
│       │   └── CTranslate2.framework
│       ├── macos-x86_64/
│       │   └── CTranslate2.framework
│       └── Info.plist
└── Package.swift
```

### Option 3: System Library (Advanced)

For users who want to use system-installed CTranslate2:

**Package.swift**:
```swift
.systemLibrary(
    name: "CTranslate2System",
    pkgConfig: "ctranslate2",
    providers: [
        .brew(["ctranslate2"]),
    ]
)
```

## Creating XCFramework

### Step 1: Build for All Platforms

```bash
#!/bin/bash
# build_xcframework.sh

# macOS arm64
cmake -B build-macos-arm64 \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON
cmake --build build-macos-arm64
cmake --install build-macos-arm64 --prefix install-macos-arm64

# macOS x86_64
cmake -B build-macos-x86_64 \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON
cmake --build build-macos-x86_64
cmake --install build-macos-x86_64 --prefix install-macos-x86_64

# iOS arm64
cmake -B build-ios-arm64 \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-ios-arm64
cmake --install build-ios-arm64 --prefix install-ios-arm64

# iOS Simulator (arm64 + x86_64)
cmake -B build-ios-simulator \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-ios-simulator
cmake --install build-ios-simulator --prefix install-ios-simulator
```

### Step 2: Create XCFramework

```bash
xcodebuild -create-xcframework \
  -library install-macos-arm64/lib/libctranslate2.dylib \
  -headers install-macos-arm64/include \
  -library install-macos-x86_64/lib/libctranslate2.dylib \
  -headers install-macos-x86_64/include \
  -library install-ios-arm64/lib/libctranslate2.a \
  -headers install-ios-arm64/include \
  -library install-ios-simulator/lib/libctranslate2.a \
  -headers install-ios-simulator/include \
  -output CTranslate2.xcframework
```

### Step 3: Create Release

```bash
# Zip the XCFramework
zip -r CTranslate2.xcframework.zip CTranslate2.xcframework

# Calculate checksum
swift package compute-checksum CTranslate2.xcframework.zip

# Create GitHub release and upload CTranslate2.xcframework.zip
```

## Recommended Project Structure

### Main Package (IArabicSpeech)

```
IArabicSpeech/
├── Sources/
│   └── IArabicSpeech/
│       ├── whisper/
│       ├── include/
│       └── *.cpp/swift files
├── Tests/
├── Package.swift  # References CTranslate2 binary target
└── README.md
```

### Separate Binary Repository (CTranslate2-Swift)

```
CTranslate2-Swift/
├── build_xcframework.sh
├── Binaries/
│   └── CTranslate2.xcframework/
├── .github/
│   └── workflows/
│       └── build.yml  # Auto-build on release
└── README.md
```

## Updated Package.swift

```swift
// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "IArabicSpeech",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        .library(
            name: "IArabicSpeech",
            targets: ["IArabicSpeech"]),
    ],
    targets: [
        .target(
            name: "IArabicSpeech",
            dependencies: ["CTranslate2"],
            cxxSettings: [
                .headerSearchPath("whisper"),
                .headerSearchPath("include"),
            ],
            linkerSettings: [
                .linkedLibrary("c++"),
                .linkedLibrary("z"),
            ]
        ),

        // Binary target - choose ONE of these options:

        // Option A: Remote XCFramework (production)
        .binaryTarget(
            name: "CTranslate2",
            url: "https://github.com/YOUR_USERNAME/CTranslate2-Swift/releases/download/v3.20.0/CTranslate2.xcframework.zip",
            checksum: "CHECKSUM_HERE"
        ),

        // Option B: Local XCFramework (development)
        // .binaryTarget(
        //     name: "CTranslate2",
        //     path: "Binaries/CTranslate2.xcframework"
        // ),

        .testTarget(
            name: "IArabicSpeechTests",
            dependencies: ["IArabicSpeech"]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
```

## For Users

Users just need to add IArabicSpeech as a dependency:

```swift
dependencies: [
    .package(url: "https://github.com/YOUR_USERNAME/IArabicSpeech.git", from: "1.0.0"),
]
```

Swift Package Manager will automatically download the appropriate CTranslate2 binary for their platform!

## Alternative: Embedded Binaries (Simple but Larger)

If you prefer to keep everything in one repository:

```
IArabicSpeech/
├── Sources/
├── Binaries/
│   └── CTranslate2.xcframework/  # ~50-100MB
├── Tests/
└── Package.swift
```

This is simpler but makes the repository larger.

## Recommendation

For **production**: Use Option 1 (separate binary repository with XCFramework)
For **development/testing**: Use Option 2 (local binary)
For **simplicity**: Use embedded binaries in main repository

The separate binary package approach is most professional and efficient!
