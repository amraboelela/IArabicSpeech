# IArabicSpeech

Arabic speech recognizer Swift package based on Faster Whisper.

## Overview

IArabicSpeech is a Swift package that provides Arabic speech recognition capabilities using the Faster Whisper model. This package is optimized for recognizing Arabic language speech with high accuracy and performance.

The package includes C++ implementations of core Whisper components ported from the AndroidArabicWhisper project:
- **Audio Processing** - WAV file loading, mel spectrogram extraction, FFT processing
- **Tokenization** - Whisper-compatible tokenizer with Arabic language support
- **Feature Extraction** - Audio feature extraction for Whisper models

## Features

- Arabic speech recognition
- Based on Faster Whisper technology
- Swift Package Manager support
- Cross-platform support (macOS/iOS)
- C++ core with Swift API wrapper
- WAV audio file processing
- Mel spectrogram extraction
- FFT-based audio analysis

## Requirements

- Swift 5.9+
- macOS 12.0+ / iOS 15.0+
- C++17 compiler

## Installation

### Swift Package Manager

Add the following to your `Package.swift` file:

```swift
dependencies: [
    .package(url: "https://github.com/amraboelela/IArabicSpeech.git", from: "1.0.0")
]
```

Or add it directly in Xcode:
1. File > Add Packages...
2. Enter the repository URL: `https://github.com/amraboelela/IArabicSpeech.git`
3. Select the version you want to use

## Usage

### Basic Speech Recognition

```swift
import IArabicSpeech

// Create recognizer instance
let recognizer = ArabicSpeechRecognizer()

// Recognize from WAV file
do {
    let text = try await recognizer.recognize(audioFilePath: "path/to/audio.wav")
    print("Recognized text: \(text)")
} catch {
    print("Recognition failed: \(error)")
}
```

### Audio Processing

```swift
import IArabicSpeech

// Load audio from WAV file
let audio = try ArabicSpeechRecognizer.AudioProcessor.loadAudio(from: "audio.wav")
print("Loaded \(audio.count) audio samples")

// Extract mel spectrogram
let melSpec = try ArabicSpeechRecognizer.AudioProcessor.extractMelSpectrogram(from: audio)
print("Extracted mel spectrogram: \(melSpec.count) bands × \(melSpec.first?.count ?? 0) frames")
```

## Building

```bash
swift build
```

## Testing

```bash
swift test
```

## Architecture

The package consists of:

### C++ Core Layer
- **whisper/whisper_audio.{h,cpp}** - Audio loading, resampling, mel spectrogram extraction
- **whisper/whisper_tokenizer.{h,cpp}** - Whisper tokenizer with Arabic support
- **whisper/fft.h** - FFT implementation (Cooley-Tukey + Bluestein's algorithm)
- **feature_extractor.{h,cpp}** - Feature extraction for Whisper models
- **IArabicSpeechBridge.cpp** - C bridge for Swift interop

### Swift API Layer
- **IArabicSpeech.swift** - Main Swift API
- **ArabicSpeechRecognizer** - High-level speech recognition interface
- **AudioProcessor** - Audio loading and feature extraction utilities

## Project Structure

```
Sources/IArabicSpeech/
├── whisper/
│   ├── whisper_audio.h/cpp     # Audio processing
│   ├── whisper_tokenizer.h/cpp # Tokenization
│   └── fft.h                   # FFT implementation
├── include/
│   ├── feature_extractor.h     # Feature extraction
│   └── IArabicSpeech-Bridging.h # C bridge header
├── feature_extractor.cpp       # Feature extraction implementation
├── IArabicSpeechBridge.cpp     # C bridge implementation
└── IArabicSpeech.swift         # Swift API
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Created by Amr Aboelela

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- Based on [Faster Whisper](https://github.com/guillaumekln/faster-whisper) by Guillaume Klein
- Ported from [AndroidArabicWhisper](https://github.com/amraboelela/AndroidArabicWhisper)
- Uses Whisper model architecture from OpenAI

