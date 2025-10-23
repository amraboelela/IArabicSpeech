// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "IArabicSpeech",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "IArabicSpeech",
            targets: ["IArabicSpeech"]),
        .library(
            name: "faster_whisper",
            targets: ["faster_whisper"]),
    ],
    dependencies: [
        // No external dependencies - CTranslate2 is embedded
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.

        // C/C++ target for Faster Whisper
        .target(
            name: "faster_whisper",
            dependencies: ["CTranslate2"],
            resources: [
                .copy("model")
            ],
            cSettings: [
                .headerSearchPath("."),
                .headerSearchPath("whisper"),
                .headerSearchPath("include"),
                .headerSearchPath("CTranslate2Headers"),
            ],
            cxxSettings: [
                .headerSearchPath("."),
                .headerSearchPath("whisper"),
                .headerSearchPath("include"),
                .headerSearchPath("CTranslate2Headers"),
            ],
            linkerSettings: [
                .linkedLibrary("c++"),
                .linkedLibrary("z"),
            ]
        ),

        // Swift target
        .target(
            name: "IArabicSpeech",
            dependencies: ["faster_whisper"]
        ),

        // Embedded CTranslate2 binary framework
        .binaryTarget(
            name: "CTranslate2",
            path: "CTranslate2.xcframework"
        ),
        
        .testTarget(
            name: "IArabicSpeechTests",
            dependencies: ["IArabicSpeech"]),
    ],
    cxxLanguageStandard: .cxx17
)
