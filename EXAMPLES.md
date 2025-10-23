# IArabicSpeech Usage Examples

This file contains practical examples of how to use IArabicSpeech for Arabic speech recognition.

## Basic Usage (Without CTranslate2)

When `NO_CTRANSLATE2` is defined (default), you can still use the audio processing utilities:

```swift
import IArabicSpeech

// Load audio from a WAV file
let audioPath = "/path/to/audio.wav"
let audio = try ArabicSpeechRecognizer.AudioProcessor.loadAudio(from: audioPath)
print("Loaded \(audio.count) audio samples")

// Extract mel spectrogram features
let melSpec = try ArabicSpeechRecognizer.AudioProcessor.extractMelSpectrogram(from: audio)
print("Extracted \(melSpec.count) mel bands × \(melSpec.first?.count ?? 0) frames")
```

## Full Transcription (With CTranslate2)

When CTranslate2 is enabled, you get full speech recognition capabilities:

### Example 1: Basic Transcription

```swift
import IArabicSpeech

// Create recognizer instance
let recognizer = ArabicSpeechRecognizer()

// Load model
let modelPath = "/Users/username/whisper-models/medium"
try recognizer.loadModel(path: modelPath)

// Transcribe audio file
let audioPath = "/path/to/arabic_speech.wav"
let text = try await recognizer.recognize(audioFilePath: audioPath)
print("Transcription: \(text)")
```

### Example 2: Transcription with Segments and Timestamps

```swift
import IArabicSpeech

let recognizer = ArabicSpeechRecognizer()
try recognizer.loadModel(path: "/Users/username/whisper-models/medium")

// Get detailed transcription with segments
let result = try await recognizer.transcribe(audioFilePath: "/path/to/audio.wav")

print("Language: \(result.language) (confidence: \(result.languageProbability))")
print("Duration: \(result.duration) seconds")
print("\nSegments:")

for segment in result.segments {
    print("[\(segment.start)s -> \(segment.end)s] \(segment.text)")
}
```

### Example 3: Transcribe with Specific Language

```swift
import IArabicSpeech

let recognizer = ArabicSpeechRecognizer()
try recognizer.loadModel(path: "/Users/username/whisper-models/medium")

// Specify Arabic language (skip auto-detection)
let text = try await recognizer.recognize(
    audioFilePath: "/path/to/audio.wav",
    language: "ar"
)
print("Arabic transcription: \(text)")

// Or use English
let englishText = try await recognizer.recognize(
    audioFilePath: "/path/to/english_audio.wav",
    language: "en"
)
print("English transcription: \(englishText)")
```

### Example 4: Process Audio from Memory

```swift
import IArabicSpeech

let recognizer = ArabicSpeechRecognizer()
try recognizer.loadModel(path: "/Users/username/whisper-models/medium")

// Load audio file into Data
let audioData = try Data(contentsOf: URL(fileURLWithPath: "/path/to/audio.wav"))

// Transcribe from Data
let text = try await recognizer.recognize(audioData: audioData)
print("Transcription: \(text)")
```

### Example 5: Batch Processing Multiple Files

```swift
import IArabicSpeech
import Foundation

let recognizer = ArabicSpeechRecognizer()
try recognizer.loadModel(path: "/Users/username/whisper-models/medium")

let audioFiles = [
    "/path/to/audio1.wav",
    "/path/to/audio2.wav",
    "/path/to/audio3.wav"
]

for audioPath in audioFiles {
    print("\nProcessing: \(audioPath)")

    let result = try await recognizer.transcribe(audioFilePath: audioPath, language: "ar")

    print("Language: \(result.language) (confidence: \(result.languageProbability))")
    print("Transcription: \(result.segments.map { $0.text }.joined(separator: " "))")
}
```

### Example 6: Real-time Streaming (Conceptual)

```swift
import IArabicSpeech
import AVFoundation

// This example shows how you might integrate with AVAudioEngine
// for near-real-time transcription

class RealtimeTranscriber {
    let recognizer = ArabicSpeechRecognizer()
    var audioBuffer: [Float] = []
    let chunkDuration: TimeInterval = 5.0  // Process every 5 seconds
    let sampleRate: Double = 16000.0

    init() throws {
        try recognizer.loadModel(path: "/Users/username/whisper-models/base")
    }

    func processAudioChunk(_ samples: [Float]) throws {
        audioBuffer.append(contentsOf: samples)

        // Process when we have enough samples
        let requiredSamples = Int(chunkDuration * sampleRate)
        if audioBuffer.count >= requiredSamples {
            let chunk = Array(audioBuffer.prefix(requiredSamples))
            audioBuffer.removeFirst(requiredSamples)

            // Transcribe chunk
            let result = try recognizer.transcribe(audio: chunk, language: "ar")
            print("Chunk transcription: \(result.segments.map { $0.text }.joined())")
        }
    }
}
```

### Example 7: Model Selection Based on Device Capabilities

```swift
import IArabicSpeech
import Foundation

class AdaptiveRecognizer {
    let recognizer = ArabicSpeechRecognizer()

    func selectAndLoadModel() throws {
        // Check system capabilities
        let processInfo = ProcessInfo.processInfo
        let memoryGB = processInfo.physicalMemory / (1024 * 1024 * 1024)
        let processorCount = processInfo.activeProcessorCount

        // Select model based on available resources
        let modelName: String
        if memoryGB >= 8 && processorCount >= 4 {
            modelName = "medium"  // Best quality
        } else if memoryGB >= 4 {
            modelName = "small"   // Good balance
        } else {
            modelName = "base"    // Faster, lower memory
        }

        let modelPath = "/Users/username/whisper-models/\(modelName)"
        try recognizer.loadModel(path: modelPath)
        print("Loaded \(modelName) model")
    }

    func transcribe(_ audioPath: String) async throws -> String {
        return try await recognizer.recognize(audioFilePath: audioPath)
    }
}
```

### Example 8: Error Handling

```swift
import IArabicSpeech

func safeTranscribe(audioPath: String) async {
    let recognizer = ArabicSpeechRecognizer()

    do {
        // Try to load model
        let modelPath = "/Users/username/whisper-models/medium"
        try recognizer.loadModel(path: modelPath)

        // Check if model is loaded
        guard recognizer.hasModelLoaded else {
            print("Error: Model not loaded")
            return
        }

        // Transcribe audio
        let result = try await recognizer.transcribe(audioFilePath: audioPath)
        print("Success: \(result.segments.map { $0.text }.joined(separator: " "))")

    } catch SpeechRecognitionError.invalidAudioData {
        print("Error: Invalid audio file format")
    } catch SpeechRecognitionError.modelNotLoaded {
        print("Error: Model not loaded. Call loadModel() first.")
    } catch SpeechRecognitionError.modelLoadFailed {
        print("Error: Failed to load model. Check model path and CTranslate2 installation.")
    } catch SpeechRecognitionError.ctranslate2NotEnabled {
        print("Error: CTranslate2 is not enabled. See CTRANSLATE2.md for setup instructions.")
    } catch {
        print("Unexpected error: \(error)")
    }
}

// Usage
await safeTranscribe(audioPath: "/path/to/audio.wav")
```

### Example 9: Transcription to Subtitles (SRT Format)

```swift
import IArabicSpeech
import Foundation

func generateSubtitles(from audioPath: String, outputPath: String) async throws {
    let recognizer = ArabicSpeechRecognizer()
    try recognizer.loadModel(path: "/Users/username/whisper-models/medium")

    let result = try await recognizer.transcribe(audioFilePath: audioPath)

    var srtContent = ""
    for (index, segment) in result.segments.enumerated() {
        let startTime = formatSRTTime(segment.start)
        let endTime = formatSRTTime(segment.end)

        srtContent += "\(index + 1)\n"
        srtContent += "\(startTime) --> \(endTime)\n"
        srtContent += "\(segment.text)\n\n"
    }

    try srtContent.write(toFile: outputPath, atomically: true, encoding: .utf8)
    print("Subtitles saved to: \(outputPath)")
}

func formatSRTTime(_ seconds: Float) -> String {
    let hours = Int(seconds) / 3600
    let minutes = (Int(seconds) % 3600) / 60
    let secs = Int(seconds) % 60
    let millis = Int((seconds - Float(Int(seconds))) * 1000)
    return String(format: "%02d:%02d:%02d,%03d", hours, minutes, secs, millis)
}

// Usage
await generateSubtitles(
    from: "/path/to/video_audio.wav",
    outputPath: "/path/to/subtitles.srt"
)
```

### Example 10: Performance Monitoring

```swift
import IArabicSpeech
import Foundation

func benchmarkTranscription(audioPath: String) async throws {
    let recognizer = ArabicSpeechRecognizer()
    try recognizer.loadModel(path: "/Users/username/whisper-models/base")

    // Load audio
    let startLoad = Date()
    let audio = try ArabicSpeechRecognizer.AudioProcessor.loadAudio(from: audioPath)
    let loadTime = Date().timeIntervalSince(startLoad)

    print("Audio loading: \(loadTime * 1000) ms")
    print("Audio duration: \(Float(audio.count) / 16000.0) seconds")

    // Transcribe
    let startTranscribe = Date()
    let result = try recognizer.transcribe(audio: audio, language: "ar")
    let transcribeTime = Date().timeIntervalSince(startTranscribe)

    print("Transcription: \(transcribeTime * 1000) ms")
    print("Real-time factor: \(transcribeTime / Double(result.duration))")
    print("Segments: \(result.segments.count)")
}

// Usage
await benchmarkTranscription(audioPath: "/path/to/audio.wav")
```

## Command-Line Tool Example

Create an executable Swift script:

```swift
#!/usr/bin/env swift

import Foundation
import IArabicSpeech

@main
struct TranscribeTool {
    static func main() async throws {
        let args = CommandLine.arguments

        guard args.count >= 3 else {
            print("Usage: transcribe <model-path> <audio-file> [language]")
            print("Example: transcribe ~/whisper-models/medium audio.wav ar")
            return
        }

        let modelPath = args[1]
        let audioPath = args[2]
        let language = args.count > 3 ? args[3] : nil

        print("Loading model from: \(modelPath)")
        let recognizer = ArabicSpeechRecognizer()
        try recognizer.loadModel(path: modelPath)

        print("Transcribing: \(audioPath)")
        let result = try await recognizer.transcribe(audioFilePath: audioPath, language: language)

        print("\nLanguage: \(result.language) (confidence: \(Int(result.languageProbability * 100))%)")
        print("Duration: \(result.duration) seconds")
        print("\nTranscription:")
        for segment in result.segments {
            print("[\(String(format: "%.2f", segment.start))s -> \(String(format: "%.2f", segment.end))s] \(segment.text)")
        }
    }
}
```

Save as `transcribe.swift` and run:

```bash
chmod +x transcribe.swift
./transcribe.swift ~/whisper-models/medium audio.wav ar
```

## Testing Models Before Integration

Before integrating into your app, test models with a simple script:

```swift
import IArabicSpeech

let modelPaths = [
    "base": "/Users/username/whisper-models/base",
    "small": "/Users/username/whisper-models/small",
    "medium": "/Users/username/whisper-models/medium"
]

let testAudio = "/path/to/test_audio.wav"

for (name, path) in modelPaths {
    print("\n=== Testing \(name) model ===")
    let recognizer = ArabicSpeechRecognizer()

    do {
        try recognizer.loadModel(path: path)
        let start = Date()
        let result = try await recognizer.transcribe(audioFilePath: testAudio)
        let elapsed = Date().timeIntervalSince(start)

        print("Time: \(elapsed)s")
        print("Text: \(result.segments.map { $0.text }.joined(separator: " "))")
    } catch {
        print("Error: \(error)")
    }
}
```

## Tips and Best Practices

1. **Model Selection**: Use smaller models (base/small) for faster inference, larger models (medium/large) for better accuracy
2. **Audio Quality**: 16kHz mono WAV files work best
3. **Error Handling**: Always wrap model loading and transcription in try-catch blocks
4. **Memory Management**: Models are automatically cleaned up when recognizer is deallocated
5. **Threading**: Transcription methods are async and can be called from any thread
6. **Batch Processing**: Reuse the same recognizer instance for multiple files to avoid reloading the model

## Troubleshooting

If transcription fails:
- Verify CTranslate2 is installed (`ls /usr/local/lib/libctranslate2.*`)
- Check model files exist (`ls ~/whisper-models/medium/*.json`)
- Ensure vocabulary.json is present in the model directory
- Test with a known-good audio file first
- Check audio format (should be WAV, 16kHz, mono)
