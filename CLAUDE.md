# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
IArabicSpeech is an Arabic speech recognizer Swift package based on Faster Whisper. This project provides speech recognition capabilities specifically optimized for Arabic language, with a C++ core and Swift API wrapper.

## Technology Stack
- Language: Swift 5.9+ with C++17 interop
- Platform: macOS/iOS (Swift-based)
- Core Technology: Faster Whisper (C++ implementation)
- Package Management: Swift Package Manager (SPM)
- Audio Processing: Custom FFT, mel spectrogram extraction
- Tokenization: Whisper-compatible tokenizer

## Development Commands

### Building the Project
```bash
swift build
```

### Running Tests
```bash
swift test
```

### Clean Build
```bash
swift package clean
```

### Update Dependencies
```bash
swift package update
```

## Project Structure

### Swift Package Layout
```
Sources/IArabicSpeech/
├── whisper/                     # C++ core audio processing
│   ├── whisper_audio.{h,cpp}   # Audio loading, mel spectrogram
│   ├── whisper_tokenizer.{h,cpp} # Tokenization with Arabic support
│   └── fft.h                    # FFT implementation
├── include/                     # C++ headers
│   ├── feature_extractor.h     # Feature extraction interface
│   ├── transcribe.h            # Whisper model interface
│   └── IArabicSpeech-Bridging.h # C bridge for Swift interop
├── feature_extractor.cpp       # Feature extraction implementation
├── transcribe.cpp              # Whisper model implementation
├── IArabicSpeechBridge.cpp     # C bridge implementation
└── IArabicSpeech.swift         # Swift API
```

### Key Components

#### C++ Core Layer
- **whisper_audio.{h,cpp}**: Audio file loading (WAV), resampling, mono/stereo conversion, mel spectrogram extraction using FFT
- **whisper_tokenizer.{h,cpp}**: Whisper GPT-2 based tokenizer with Arabic language support (50+ languages)
- **fft.h**: FFT implementation using Cooley-Tukey (power of 2) and Bluestein's algorithm (arbitrary sizes)
- **feature_extractor.{h,cpp}**: Audio feature extraction matching Faster Whisper's preprocessing
- **transcribe.{h,cpp}**: Whisper model interface for transcription (placeholder for future CTranslate2 integration)
- **IArabicSpeechBridge.cpp**: C bridge layer for Swift-C++ interop

#### Swift API Layer
- **IArabicSpeech.swift**: Main public API exposing:
  - `ArabicSpeechRecognizer`: High-level speech recognition interface
  - `AudioProcessor`: Audio loading and mel spectrogram extraction utilities

## Architecture Details

### Audio Processing Pipeline
1. **Load WAV file** → `whisper::AudioProcessor::load_audio()`
2. **Convert to mono** (if stereo) → `stereo_to_mono()`
3. **Resample to 16kHz** (if needed) → `resample_audio()`
4. **Extract mel spectrogram** → `extract_mel_spectrogram()`
   - Apply Hann window
   - Compute STFT using FFT
   - Apply mel filter bank (80 mel bands)
   - Apply log transform
   - Normalize to [-1, ~1.5] range

### FFT Implementation
- Uses **Cooley-Tukey algorithm** for power-of-2 sizes (O(N log N))
- Uses **Bluestein's algorithm** for arbitrary sizes (converts to circular convolution)
- Supports real FFT (rfft) returning only positive frequencies

### Tokenization
- Based on GPT-2 BPE tokenizer
- Supports 50+ languages including Arabic (language code: "ar", token ID: 50272)
- Special tokens: SOT, EOT, transcribe, translate, timestamps, etc.
- Handles Arabic Unicode properly with UTF-8 encoding

## Swift Coding Style
- Use Swift naming conventions (camelCase for variables/functions, PascalCase for types)
- Prefer `if let` unwrapping without redundant variable names (use `if let handler {` instead of `if let handler = handler {`)
- Use `async/await` for asynchronous operations
- Document public APIs with triple-slash comments (`///`)
- Created by Amr Aboelela (not "Created by Claude")

## C++ Coding Style
- Use 2-space indentation
- Follow RAII patterns for memory management
- Use `std::vector`, `std::unique_ptr`, etc. for automatic memory management
- Document complex algorithms
- Namespace whisper components under `namespace whisper`

## Git Workflow
- Do NOT commit changes automatically
- Make requested code changes and run tests
- Fix any linting issues
- Leave all changes uncommitted for manual review and commit by the user

## Key Technical Details

### Audio Constants (matching Whisper spec)
```cpp
WHISPER_SAMPLE_RATE = 16000    // 16kHz audio
WHISPER_N_FFT = 400            // FFT window size
WHISPER_HOP_LENGTH = 160       // 10ms hop (16000 / 160 = 100 Hz frame rate)
WHISPER_CHUNK_SIZE = 480000    // 30 seconds at 16kHz
WHISPER_N_MEL = 80             // 80 mel filter banks
```

### Mel Spectrogram Processing
- Uses **Slaney-style mel scale** (not HTK)
- Piecewise linear/logarithmic frequency mapping
- Frequency range: 0 Hz to 8 kHz (Nyquist at 16kHz sampling)
- Log10 transform with clamping: `max(log_spec, log_spec.max() - 8.0)`
- Final normalization: `(log_spec + 4.0) / 4.0`

### C-Swift Bridging
- C bridge header: `IArabicSpeech-Bridging.h`
- Exposes C functions for Swift interop
- Manual memory management:
  - Allocate in C++: `malloc()`
  - Copy data from C++ containers
  - Free in Swift: `whisper_free_*()` functions
- Use `extern "C"` to prevent C++ name mangling

## Common Issues and Solutions

### Build Issues
1. **C++ standard version**: Ensure C++17 is specified in Package.swift
2. **Header paths**: Use `.headerSearchPath()` in Package.swift for whisper/ and include/
3. **Linker errors**: Add `.linkedLibrary("c++")` to link C++ standard library

### Memory Management
- Always free C-allocated memory in Swift using provided free functions
- Use `UnsafeBufferPointer` to safely access C arrays from Swift
- Convert C arrays to Swift arrays early, then free C memory immediately

### Audio Processing
- Only support 16-bit PCM WAV files currently
- Normalize audio to [-1, 1] range when converting from int16
- Apply center padding for STFT to match Python's `center=True` behavior

## Testing Strategy
- Unit test Swift API with sample WAV files
- Test audio loading and mel spectrogram extraction
- Verify memory cleanup (no leaks)
- Test edge cases: empty files, invalid formats, very short audio

## Future Development
- [ ] Integrate CTranslate2 for actual Whisper model inference
- [ ] Add model downloading and caching
- [ ] Implement full transcription pipeline
- [ ] Add word-level timestamps
- [ ] Support more audio formats (MP3, M4A)
- [ ] Add real-time streaming recognition
- [ ] Optimize performance with Metal/Accelerate frameworks

## Dependencies (Future)
- **CTranslate2**: Fast Transformer model inference (currently disabled with NO_CTRANSLATE2)
- Models from Hugging Face or OpenAI Whisper checkpoints

## Source Attribution
- Ported from [AndroidArabicWhisper](https://github.com/amraboelela/AndroidArabicWhisper)
- Based on [Faster Whisper](https://github.com/guillaumekln/faster-whisper) Python implementation
- Whisper architecture from OpenAI

