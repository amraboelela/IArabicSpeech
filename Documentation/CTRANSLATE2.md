# CTranslate2 Integration Guide

This document explains how to integrate CTranslate2 with IArabicSpeech to enable full Whisper model transcription capabilities.

## Overview

IArabicSpeech uses CTranslate2 as its inference engine for Whisper models. CTranslate2 provides:
- Fast CPU inference with optimized operations
- Support for quantized models (INT8, FLOAT16, FLOAT32)
- Efficient beam search and generation
- Memory-optimized model loading

## Installation

### Prerequisites

- macOS 10.15+ (Catalina or later)
- Xcode 14.0+
- CMake 3.15+
- Homebrew (recommended for dependencies)

### Step 1: Install Dependencies

```bash
# Install build tools
brew install cmake

# Install OpenBLAS for optimized linear algebra
brew install openblas

# Optional: Install Intel MKL for better performance on Intel CPUs
# brew install intel-mkl
```

### Step 2: Build CTranslate2

```bash
# Clone CTranslate2 repository
git clone https://github.com/OpenNMT/CTranslate2.git
cd CTranslate2

# Create build directory
mkdir build
cd build

# Configure with OpenBLAS (recommended for macOS)
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DWITH_OPENBLAS=ON \
  -DWITH_MKL=OFF \
  -DOPENMP_RUNTIME=NONE \
  -DBUILD_CLI=OFF

# Build (use -j to parallelize, e.g., -j8 for 8 cores)
make -j8

# Install system-wide
sudo make install
```

### Step 3: Verify Installation

```bash
# Check if CTranslate2 library is installed
ls -l /usr/local/lib/libctranslate2.*

# Expected output:
# libctranslate2.a
# libctranslate2.dylib
```

## Configuring IArabicSpeech

### Step 1: Remove NO_CTRANSLATE2 Flag

Edit `Package.swift` to remove the `NO_CTRANSLATE2` preprocessor flag:

```swift
// Remove or comment out this line:
// .define("NO_CTRANSLATE2"),

cxxSettings: [
    .headerSearchPath("whisper"),
    .headerSearchPath("include"),
    // .define("NO_CTRANSLATE2"),  // <-- REMOVE THIS
],
```

### Step 2: Add CTranslate2 Linker Settings

Add CTranslate2 library paths to `Package.swift`:

```swift
linkerSettings: [
    .linkedLibrary("c++"),
    .linkedLibrary("ctranslate2"),
    .unsafeFlags(["-L/usr/local/lib"]),
]
```

### Step 3: Update Header Search Paths

Add CTranslate2 include path:

```swift
cxxSettings: [
    .headerSearchPath("whisper"),
    .headerSearchPath("include"),
    .unsafeFlags(["-I/usr/local/include"]),
],
```

## Downloading Whisper Models

### Using Faster-Whisper Models

CTranslate2-optimized Whisper models are available on Hugging Face:

```bash
# Install Hugging Face CLI
pip install huggingface-hub

# Download a model (e.g., faster-whisper-base)
huggingface-cli download Systran/faster-whisper-base --local-dir ~/whisper-models/base

# Download Arabic-optimized model (if available)
# huggingface-cli download Systran/faster-whisper-medium --local-dir ~/whisper-models/medium
```

### Available Models

| Model | Size | Languages | Recommended For |
|-------|------|-----------|-----------------|
| `faster-whisper-tiny` | ~75MB | 99+ | Testing, embedded devices |
| `faster-whisper-base` | ~145MB | 99+ | Mobile apps, quick transcription |
| `faster-whisper-small` | ~488MB | 99+ | General purpose |
| `faster-whisper-medium` | ~1.5GB | 99+ | Arabic speech (recommended) |
| `faster-whisper-large-v3` | ~3GB | 99+ | Maximum accuracy |

### Model Directory Structure

After downloading, your model directory should contain:

```
~/whisper-models/base/
├── config.json              # Model configuration
├── model.bin                # CTranslate2 model weights
├── vocabulary.json          # Token vocabulary (required!)
├── tokenizer.json           # Tokenizer configuration
└── preprocessor_config.json # Audio preprocessing config
```

**Important**: The `vocabulary.json` file is required for the tokenizer to work properly.

## Usage Example

### Swift API

```swift
import IArabicSpeech

// Initialize model
let recognizer = ArabicSpeechRecognizer()

// Load model
let modelPath = "~/whisper-models/medium"
try await recognizer.loadModel(path: modelPath)

// Transcribe audio
let audioPath = "path/to/audio.wav"
let result = try await recognizer.recognize(audioFilePath: audioPath)

print("Transcription: \(result)")
```

### C++ API (Advanced)

```cpp
#include "include/transcribe.h"
#include "whisper/whisper_audio.h"

using namespace whisper;

// Initialize model
WhisperModel model(
    "/path/to/model",      // model_path
    "cpu",                  // device
    {},                     // device_index
    "float32",              // compute_type
    0,                      // cpu_threads (0 = auto)
    1                       // num_workers
);

// Load audio
auto audio = AudioProcessor::load_audio("audio.wav");

// Transcribe
auto [segments, info] = model.transcribe(
    audio,
    "ar",    // language (optional, auto-detect if not provided)
    true     // multilingual
);

// Print results
for (const auto& seg : segments) {
    std::cout << "[" << seg.start << "s -> " << seg.end << "s] "
              << seg.text << std::endl;
}
```

## Performance Tuning

### Compute Types

CTranslate2 supports different compute types with trade-offs:

| Type | Speed | Accuracy | Memory | Requirements |
|------|-------|----------|--------|--------------|
| `INT8` | Fastest | Good | Lowest | AVX512 VNNI (Intel Ice Lake+) |
| `FLOAT16` | Fast | Very Good | Low | Apple Silicon, ARM64 |
| `FLOAT32` | Medium | Best | Highest | All CPUs |

**Note**: On macOS without AVX512, use `FLOAT32` (default) or `FLOAT16` (Apple Silicon).

### Thread Configuration

```cpp
// Set number of CPU threads (0 = auto-detect)
WhisperModel model(
    model_path,
    "cpu",
    {},
    "float32",
    4,        // cpu_threads: 4 threads for parallel processing
    1         // num_workers: 1 model instance
);
```

### Memory Optimization

For lower memory usage:
1. Use smaller models (tiny, base, small)
2. Use INT8 quantization if supported
3. Limit beam size: `options.beam_size = 3` (default is 5)

## Troubleshooting

### Build Errors

**Error**: `fatal error: 'ctranslate2/models/whisper.h' file not found`

**Solution**: Ensure CTranslate2 is installed and include path is correct:
```bash
ls /usr/local/include/ctranslate2/
```

**Error**: `library not found for -lctranslate2`

**Solution**: Check library installation and update linker paths:
```bash
ls /usr/local/lib/libctranslate2.*
```

### Runtime Errors

**Error**: `Failed to open vocabulary file`

**Solution**: Ensure the model directory contains `vocabulary.json`:
```bash
ls ~/whisper-models/base/vocabulary.json
```

**Error**: `CTranslate2 rejected INT8`

**Solution**: Use `FLOAT32` compute type instead of `INT8` if your CPU doesn't support AVX512 VNNI.

### Performance Issues

**Slow transcription**:
1. Check CPU usage with Activity Monitor
2. Increase `cpu_threads` parameter
3. Use a smaller model for faster inference
4. Consider using INT8 quantization if supported

## Converting Original Whisper Models

If you have OpenAI Whisper models, convert them to CTranslate2 format:

```bash
# Install ct2-transformers-converter
pip install ctranslate2

# Convert model
ct2-transformers-converter --model openai/whisper-medium \
    --output_dir ~/whisper-models/medium \
    --quantization float32

# Add vocabulary.json (required for tokenizer)
# Download from Systran models or extract from original model
```

## Further Resources

- [CTranslate2 Documentation](https://opennmt.net/CTranslate2/)
- [Faster-Whisper Python Library](https://github.com/SYSTRAN/faster-whisper)
- [Whisper Model Card](https://huggingface.co/openai/whisper-medium)
- [Systran Faster-Whisper Models](https://huggingface.co/Systran)

## License

CTranslate2 is licensed under the MIT License. Whisper models are licensed under the MIT License by OpenAI.
