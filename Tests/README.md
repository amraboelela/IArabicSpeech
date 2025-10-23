# IArabicSpeech Tests

This directory contains both Swift and C++ tests for IArabicSpeech.

## Test Structure

```
Tests/
├── IArabicSpeechTests/          # Swift tests using Testing framework
│   └── IArabicSpeechTests.swift
└── Cpp/                          # C++ tests (direct component testing)
    ├── CMakeLists.txt
    ├── run_tests.sh
    ├── feature_extractor_tests.cpp
    ├── whisper_audio_tests.cpp
    └── whisper_tokenizer_tests.cpp
```

## Running Swift Tests

```bash
# Run all Swift tests
swift test

# Run specific test
swift test --filter IArabicSpeechTests
```

## Running C++ Tests

C++ tests directly test the C++ components (feature extraction, audio processing, tokenization):

```bash
cd Tests/Cpp
./run_tests.sh
```

Or manually:

```bash
cd Tests/Cpp
mkdir build && cd build
cmake ..
make
./feature_extractor_tests
./whisper_audio_tests
./whisper_tokenizer_tests
```

## Test Coverage

### Swift Tests
- Audio loading from WAV files
- Mel spectrogram extraction
- Model loading (CTranslate2)
- Full transcription pipeline
- Language detection
- Error handling

### C++ Tests
- Feature extractor initialization
- Mel filter bank generation
- STFT computation
- Mel spectrogram computation with chunking
- Audio file processing (real files)
- Whisper tokenizer functionality
- Audio preprocessing

## Requirements

- Swift 5.9+ (for Swift tests)
- CMake 3.15+ (for C++ tests)
- CTranslate2 library bundled in package

## Notes

- Swift tests use the new Testing framework (not XCTest)
- C++ tests are adapted from the Android project
- Tests automatically skip when models are not available
