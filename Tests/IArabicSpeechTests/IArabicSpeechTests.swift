///
/// IArabicSpeechTests.swift
/// IArabicSpeech Tests
///
/// Created by Amr Aboelela on 10/21/2025.
///

import Testing
import Foundation
@testable import IArabicSpeech

@Suite("Arabic Speech Recognition Tests")
struct IArabicSpeechTests {

    // MARK: - Audio Processing Tests

    @Test("Load audio from WAV file")
    func testAudioLoading() throws {
        print("=== Audio Loading Test ===")

        // Create a synthetic WAV file for testing
        let testAudioPath = try createSyntheticWAVFile()

        do {
            let audio = try IArabicSpeechRecognizer.AudioProcessor.loadAudio(from: testAudioPath)

            #expect(!audio.isEmpty, "Audio should not be empty")
            #expect(audio.count > 0, "Audio should have samples")

            print("✓ Successfully loaded \(audio.count) audio samples")

            // Verify sample values are in valid range [-1.0, 1.0]
            let maxValue = audio.max() ?? 0.0
            let minValue = audio.min() ?? 0.0

            #expect(maxValue <= 1.0, "Max audio value should be <= 1.0")
            #expect(minValue >= -1.0, "Min audio value should be >= -1.0")

            print("  - Min value: \(minValue)")
            print("  - Max value: \(maxValue)")
            print("  - Duration: \(Float(audio.count) / 16000.0) seconds")

        } catch {
            Issue.record("Failed to load audio: \(error)")
        }

        // Clean up
        try? FileManager.default.removeItem(atPath: testAudioPath)
    }

    @Test("Extract mel spectrogram from audio")
    func testMelSpectrogramExtraction() throws {
        print("\n=== Mel Spectrogram Extraction Test ===")

        // Create synthetic audio (2 seconds at 16kHz)
        let sampleRate: Float = 16000.0
        let duration: Float = 2.0
        let frequency: Float = 440.0  // A4 note

        let numSamples = Int(duration * sampleRate)
        var audio: [Float] = []

        for i in 0..<numSamples {
            let t = Float(i) / sampleRate
            let sample = 0.5 * sin(2.0 * .pi * frequency * t)
            audio.append(sample)
        }

        print("Generated \(audio.count) audio samples")

        do {
            let melSpec = try IArabicSpeechRecognizer.AudioProcessor.extractMelSpectrogram(from: audio)

            #expect(!melSpec.isEmpty, "Mel spectrogram should not be empty")
            #expect(melSpec.count > 0, "Should have mel bands")
            #expect(melSpec.first?.count ?? 0 > 0, "Should have time frames")

            print("✓ Extracted mel spectrogram: \(melSpec.count) mel bands × \(melSpec.first?.count ?? 0) frames")

            // Verify dimensions (should be 80 mel bands for Whisper)
            #expect(melSpec.count == 80, "Should have 80 mel bands")

        } catch {
            Issue.record("Failed to extract mel spectrogram: \(error)")
        }
    }

    @Test("Complete audio processing pipeline")
    func testAudioProcessingPipeline() throws {
        print("\n=== Audio Processing Pipeline Test ===")

        // Create test audio file
        let testPath = try createSyntheticWAVFile()
        defer { try? FileManager.default.removeItem(atPath: testPath) }

        do {
            // Step 1: Load audio
            let audio = try IArabicSpeechRecognizer.AudioProcessor.loadAudio(from: testPath)
            print("✓ Step 1: Loaded \(audio.count) samples")

            // Step 2: Extract mel spectrogram
            let melSpec = try IArabicSpeechRecognizer.AudioProcessor.extractMelSpectrogram(from: audio)
            print("✓ Step 2: Extracted \(melSpec.count)×\(melSpec.first?.count ?? 0) features")

            // Verify complete pipeline
            #expect(!audio.isEmpty)
            #expect(!melSpec.isEmpty)

            print("✓ Audio processing pipeline completed successfully")

        } catch {
            Issue.record("Pipeline failed: \(error)")
        }
    }

    // MARK: - Model Loading Tests (CTranslate2)

    @Test("Load Whisper model")
    func testModelLoading() throws {
        print("\n=== Model Loading Test ===")

        let recognizer = IArabicSpeechRecognizer()

        // Test that model is not loaded initially
        #expect(!recognizer.hasModelLoaded, "Model should not be loaded initially")

        // Use bundled model resource
        guard let modelPath = IArabicSpeechRecognizer.bundledModelPath() else {
            throw XCTSkip("Model resource not found in bundle")
        }

        print("Model path: \(modelPath)")

        do {
            try recognizer.loadModel(path: modelPath)
            #expect(recognizer.hasModelLoaded, "Model should be loaded after loadModel()")
            print("✓ Model loaded successfully from \(modelPath)")
        } catch SpeechRecognitionError.modelLoadFailed {
            print("⚠ Model loading failed - ensure model exists at \(modelPath)")
            throw XCTSkip("Model not available at expected path")
        } catch {
            Issue.record("Unexpected error loading model: \(error)")
        }
    }

    @Test("Transcribe audio with model")
    func testTranscription() async throws {
        print("\n=== Transcription Test ===")

        let recognizer = IArabicSpeechRecognizer()

        // Use bundled model resource
        guard let modelPath = IArabicSpeechRecognizer.bundledModelPath() else {
            throw XCTSkip("Model resource not found in bundle")
        }

        try recognizer.loadModel(path: modelPath)
        print("✓ Model loaded")

        // Create test audio
        let testPath = try createSyntheticWAVFile()
        defer { try? FileManager.default.removeItem(atPath: testPath) }

        do {
            let result = try await recognizer.transcribe(audioFilePath: testPath, language: "ar")

            #expect(!result.segments.isEmpty, "Should have transcription segments")
            #expect(result.duration > 0, "Duration should be positive")
            #expect(!result.language.isEmpty, "Language should be detected")

            print("✓ Transcription completed:")
            print("  - Language: \(result.language) (confidence: \(result.languageProbability))")
            print("  - Duration: \(result.duration) seconds")
            print("  - Segments: \(result.segments.count)")

            for (index, segment) in result.segments.enumerated() {
                print("  [\(segment.start)s -> \(segment.end)s] \(segment.text)")
                if index >= 3 { break }
            }

        } catch {
            Issue.record("Transcription failed: \(error)")
        }
    }

    @Test("Recognize with specific language")
    func testRecognizeWithLanguage() async throws {
        print("\n=== Recognize with Language Test ===")

        let recognizer = IArabicSpeechRecognizer()

        guard let modelPath = IArabicSpeechRecognizer.bundledModelPath() else {
            throw XCTSkip("Model resource not found in bundle")
        }

        try recognizer.loadModel(path: modelPath)

        let testPath = try createSyntheticWAVFile()
        defer { try? FileManager.default.removeItem(atPath: testPath) }

        // Test Arabic
        let arabicText = try await recognizer.recognize(audioFilePath: testPath, language: "ar")
        #expect(!arabicText.isEmpty, "Arabic transcription should not be empty")
        print("✓ Arabic transcription: \(arabicText)")

        // Test English
        let englishText = try await recognizer.recognize(audioFilePath: testPath, language: "en")
        #expect(!englishText.isEmpty, "English transcription should not be empty")
        print("✓ English transcription: \(englishText)")
    }

    @Test("Auto-detect language")
    func testAutoLanguageDetection() async throws {
        print("\n=== Auto Language Detection Test ===")

        let recognizer = IArabicSpeechRecognizer()

        guard let modelPath = IArabicSpeechRecognizer.bundledModelPath() else {
            throw XCTSkip("Model resource not found in bundle")
        }

        try recognizer.loadModel(path: modelPath)

        let testPath = try createSyntheticWAVFile()
        defer { try? FileManager.default.removeItem(atPath: testPath) }

        // Test auto-detection (no language specified)
        let result = try await recognizer.transcribe(audioFilePath: testPath)

        #expect(!result.language.isEmpty, "Language should be auto-detected")
        #expect(result.languageProbability > 0, "Language confidence should be positive")

        print("✓ Auto-detected language: \(result.language) (confidence: \(result.languageProbability))")
    }

    // MARK: - Error Handling Tests

    @Test("Handle invalid audio path")
    func testInvalidAudioPath() throws {
        print("\n=== Invalid Audio Path Test ===")

        #expect(throws: SpeechRecognitionError.self) {
            try IArabicSpeechRecognizer.AudioProcessor.loadAudio(from: "/nonexistent/file.wav")
        }
        print("✓ Correctly threw error for invalid path")
    }

    @Test("Handle empty audio")
    func testEmptyAudio() throws {
        print("\n=== Empty Audio Test ===")

        let emptyAudio: [Float] = []

        #expect(throws: SpeechRecognitionError.self) {
            try IArabicSpeechRecognizer.AudioProcessor.extractMelSpectrogram(from: emptyAudio)
        }
        print("✓ Correctly threw error for empty audio")
    }

    // MARK: - Helper Methods

    private func createSyntheticWAVFile() throws -> String {
        // Create a temporary WAV file with synthetic audio
        let tempDir = NSTemporaryDirectory()
        let fileName = "test_audio_\(UUID().uuidString).wav"
        let filePath = (tempDir as NSString).appendingPathComponent(fileName)

        // Generate 2 seconds of 440Hz sine wave
        let sampleRate: Float = 16000.0
        let duration: Float = 2.0
        let frequency: Float = 440.0

        let numSamples = Int(duration * sampleRate)
        var samples: [Float] = []

        for i in 0..<numSamples {
            let t = Float(i) / sampleRate
            let sample = 0.5 * sin(2.0 * .pi * frequency * t)
            samples.append(sample)
        }

        // Write WAV file
        try writeWAVFile(samples: samples, sampleRate: Int(sampleRate), path: filePath)

        return filePath
    }

    private func writeWAVFile(samples: [Float], sampleRate: Int, path: String) throws {
        var data = Data()

        // RIFF header
        data.append("RIFF".data(using: .ascii)!)

        let dataSize = samples.count * 2
        let fileSize = 36 + dataSize
        data.append(UInt32(fileSize).littleEndianBytes)

        data.append("WAVE".data(using: .ascii)!)

        // Format chunk
        data.append("fmt ".data(using: .ascii)!)
        data.append(UInt32(16).littleEndianBytes)
        data.append(UInt16(1).littleEndianBytes)
        data.append(UInt16(1).littleEndianBytes)
        data.append(UInt32(sampleRate).littleEndianBytes)
        data.append(UInt32(sampleRate * 2).littleEndianBytes)
        data.append(UInt16(2).littleEndianBytes)
        data.append(UInt16(16).littleEndianBytes)

        // Data chunk
        data.append("data".data(using: .ascii)!)
        data.append(UInt32(dataSize).littleEndianBytes)

        // Convert float samples to 16-bit PCM
        for sample in samples {
            let pcmSample = Int16(max(min(sample, 1.0), -1.0) * 32767.0)
            data.append(pcmSample.littleEndianBytes)
        }

        try data.write(to: URL(fileURLWithPath: path))
    }
}

// MARK: - Extensions for Binary Data

extension UInt32 {
    var littleEndianBytes: Data {
        var value = self.littleEndian
        return Data(bytes: &value, count: MemoryLayout<UInt32>.size)
    }
}

extension UInt16 {
    var littleEndianBytes: Data {
        var value = self.littleEndian
        return Data(bytes: &value, count: MemoryLayout<UInt16>.size)
    }
}

extension Int16 {
    var littleEndianBytes: Data {
        var value = self.littleEndian
        return Data(bytes: &value, count: MemoryLayout<Int16>.size)
    }
}

// XCTSkip compatibility
struct XCTSkip: Error {
    let message: String
    init(_ message: String) {
        self.message = message
    }
}
