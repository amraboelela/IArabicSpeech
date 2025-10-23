# CMake configuration for IArabicSpeech C++ tests
cmake_minimum_required(VERSION 3.15)
project(IArabicSpeechTests)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../../Sources/faster_whisper/include
    ${CMAKE_SOURCE_DIR}/../../Sources/faster_whisper
    ${CMAKE_SOURCE_DIR}/../../Sources/faster_whisper/CTranslate2Headers
)

# Link directories and framework path
set(CTRANSLATE2_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/../../CTranslate2.xcframework/macos-arm64/CTranslate2.framework")
link_directories(
    ${CTRANSLATE2_FRAMEWORK_PATH}/Versions/A
)

# Add framework link flag and rpath for runtime loading
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F ${CMAKE_SOURCE_DIR}/../../CTranslate2.xcframework/macos-arm64 -framework CTranslate2")
set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/../../CTranslate2.xcframework/macos-arm64")
set(CMAKE_INSTALL_RPATH "${CMAKE_SOURCE_DIR}/../../CTranslate2.xcframework/macos-arm64")


# Source files from faster_whisper
set(IARABICSPEECH_SOURCES
    ../../Sources/faster_whisper/feature_extractor.cpp
    ../../Sources/faster_whisper/whisper/whisper_audio.cpp
    ../../Sources/faster_whisper/whisper/whisper_tokenizer.cpp
    ../../Sources/faster_whisper/tokenizer.cpp
)

# Test executables

# Feature Extractor Tests
add_executable(feature_extractor_tests
    feature_extractor_tests.cpp
    ${IARABICSPEECH_SOURCES}
)
target_link_libraries(feature_extractor_tests
    z
    c++
)

# Audio Tests
add_executable(whisper_audio_tests
    whisper_audio_tests.cpp
    ${IARABICSPEECH_SOURCES}
)
target_link_libraries(whisper_audio_tests
    z
    c++
)

# Tokenizer Tests
add_executable(whisper_tokenizer_tests
    whisper_tokenizer_tests.cpp
    ${IARABICSPEECH_SOURCES}
)
target_link_libraries(whisper_tokenizer_tests
    z
    c++
)

# Enable testing
enable_testing()

add_test(NAME FeatureExtractorTests COMMAND feature_extractor_tests)
add_test(NAME AudioTests COMMAND whisper_audio_tests)
add_test(NAME TokenizerTests COMMAND whisper_tokenizer_tests)
