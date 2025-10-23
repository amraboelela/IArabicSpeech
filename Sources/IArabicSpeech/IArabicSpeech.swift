///
/// IArabicSpeech.swift
/// Arabic speech recognizer based on Faster Whisper
///
/// Created by Amr Aboelela on 10/21/2025.
///

import Foundation

// Import C functions from faster_whisper module
@_exported import faster_whisper

/// Main speech recognizer class for Arabic language
public class ArabicSpeechRecognizer {

    private var modelHandle: WhisperModelHandle?
    private var isModelLoaded = false

    /// Audio processing utility
    public struct AudioProcessor {

        /// Load audio from a WAV file
        /// - Parameter filePath: Path to the WAV file
        /// - Returns: Audio samples as float array
        /// - Throws: Error if file cannot be loaded
        public static func loadAudio(from filePath: String) throws -> [Float] {
            let cPath = (filePath as NSString).utf8String
            guard let path = cPath else {
                throw SpeechRecognitionError.invalidAudioData
            }

            let result = whisper_load_audio(path)
            guard result.data != nil, result.length > 0 else {
                throw SpeechRecognitionError.invalidAudioData
            }

            // Convert to Swift array
            let audioArray = Array(UnsafeBufferPointer<Float>(start: result.data, count: Int(result.length)))

            // Free C memory
            var mutableResult = result
            whisper_free_float_array(mutableResult)

            return audioArray
        }

        /// Extract mel spectrogram from audio samples
        /// - Parameter audio: Audio samples
        /// - Returns: Mel spectrogram as 2D array
        /// - Throws: Error if extraction fails
        public static func extractMelSpectrogram(from audio: [Float]) throws -> [[Float]] {
            let result = whisper_extract_mel_spectrogram(audio, UInt(audio.count))
            guard result.data != nil, result.rows > 0, result.cols > 0 else {
                throw SpeechRecognitionError.recognitionFailed
            }

            // Convert to Swift 2D array
            var spectrogram: [[Float]] = []
            for i in 0..<Int(result.rows) {
                if let rowPointer = result.data[Int(i)] {
                    let row = Array(UnsafeBufferPointer<Float>(start: rowPointer, count: Int(result.cols)))
                    spectrogram.append(row)
                }
            }

            // Free C memory
            var mutableResult = result
            whisper_free_float_matrix(mutableResult)

            return spectrogram
        }
    }

    /// Transcription segment with timestamp information
    public struct TranscriptionSegment {
        public let text: String
        public let start: Float
        public let end: Float
    }

    /// Complete transcription result
    public struct TranscriptionResult {
        public let segments: [TranscriptionSegment]
        public let language: String
        public let languageProbability: Float
        public let duration: Float
    }

    /// Initialize the Arabic speech recognizer
    public init() {
        modelHandle = nil
    }

    /// Get the path to the bundled Whisper model
    /// - Returns: Path to the whisper_ct2 model directory, or nil if not found
    public static func bundledModelPath() -> String? {
        // The model is bundled with the faster_whisper module inside model/whisper_ct2
        // Try to locate it in the build/test bundle structure

        // Get the path to the current executable/test bundle
        let currentBundle = Bundle.main
        let bundleURL = currentBundle.bundleURL

        // Common locations where SPM places module resources
        let possibleLocations = [
            // Direct in bundle
            bundleURL.appendingPathComponent("model/whisper_ct2"),
            // In Resources subdirectory
            bundleURL.appendingPathComponent("Contents/Resources/model/whisper_ct2"),
            bundleURL.appendingPathComponent("Resources/model/whisper_ct2"),
            // In module-specific bundle (SPM creates these for resources)
            bundleURL.deletingLastPathComponent().appendingPathComponent("faster_whisper_faster_whisper.bundle/Contents/Resources/model/whisper_ct2"),
            bundleURL.deletingLastPathComponent().appendingPathComponent("faster_whisper.bundle/Contents/Resources/model/whisper_ct2"),
            // Build directory structure
            bundleURL.deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("faster_whisper_faster_whisper.bundle/Contents/Resources/model/whisper_ct2"),
        ]

        for location in possibleLocations {
            let path = location.path
            if FileManager.default.fileExists(atPath: path) {
                return path
            }
        }

        return nil
    }

    deinit {
        // Clean up model if loaded
        if let handle = modelHandle {
            whisper_destroy_model(handle)
        }
    }

    /// Load a Whisper model from disk
    /// - Parameter modelPath: Path to the CTranslate2 model directory
    /// - Throws: Error if model cannot be loaded
    public func loadModel(path modelPath: String) throws {
        // Clean up existing model if any
        if let handle = modelHandle {
            whisper_destroy_model(handle)
            modelHandle = nil
            isModelLoaded = false
        }

        // Load new model
        let cPath = (modelPath as NSString).utf8String
        guard let path = cPath else {
            throw SpeechRecognitionError.invalidModelPath
        }

        guard let handle = whisper_create_model(path) else {
            throw SpeechRecognitionError.modelLoadFailed
        }

        modelHandle = handle
        isModelLoaded = true
    }

    /// Check if a model is loaded
    public var hasModelLoaded: Bool {
        return isModelLoaded
    }

    /// Transcribe speech from audio data with detailed results
    /// - Parameters:
    ///   - audio: Audio samples as float array
    ///   - language: Optional language code (e.g., "ar", "en"). If nil, auto-detect.
    /// - Returns: Transcription result with segments and metadata
    /// - Throws: Error if transcription fails
    public func transcribe(audio: [Float], language: String? = nil) throws -> TranscriptionResult {
        guard isModelLoaded, let handle = modelHandle else {
            throw SpeechRecognitionError.modelNotLoaded
        }

        // Perform transcription
        let cLanguage: UnsafePointer<CChar>?
        if let language = language {
            cLanguage = (language as NSString).utf8String
        } else {
            cLanguage = nil
        }
        var result = whisper_transcribe(handle, audio, UInt(audio.count), cLanguage)

        // Convert C result to Swift
        var segments: [TranscriptionSegment] = []
        if let segmentPtr = result.segments {
            for i in 0..<Int(result.segment_count) {
                let seg = segmentPtr[i]
                if let text = seg.text {
                    let segment = TranscriptionSegment(
                        text: String(cString: text),
                        start: seg.start,
                        end: seg.end
                    )
                    segments.append(segment)
                }
            }
        }

        let languageString = result.language != nil ? String(cString: result.language!) : "unknown"

        let transcriptionResult = TranscriptionResult(
            segments: segments,
            language: languageString,
            languageProbability: result.language_probability,
            duration: result.duration
        )

        // Free C memory
        whisper_free_transcription_result(result)

        return transcriptionResult
    }

    /// Recognize speech from audio data (returns combined text)
    /// - Parameters:
    ///   - audioData: The audio data to process
    ///   - language: Optional language code. If nil, auto-detect.
    /// - Returns: Recognized text in Arabic
    public func recognize(audioData: Data, language: String? = nil) async throws -> String {
        // Convert Data to Float array
        let floatArray = audioData.withUnsafeBytes { (ptr: UnsafeRawBufferPointer) -> [Float] in
            let buffer = ptr.bindMemory(to: Float.self)
            return Array(buffer)
        }

        let result = try transcribe(audio: floatArray, language: language)
        return result.segments.map { $0.text }.joined(separator: " ")
    }

    /// Recognize speech from a WAV file
    /// - Parameters:
    ///   - audioFilePath: Path to the WAV audio file
    ///   - language: Optional language code. If nil, auto-detect.
    /// - Returns: Recognized text in Arabic
    public func recognize(audioFilePath: String, language: String? = nil) async throws -> String {
        // Load audio
        let audio = try AudioProcessor.loadAudio(from: audioFilePath)

        let result = try transcribe(audio: audio, language: language)
        return result.segments.map { $0.text }.joined(separator: " ")
    }

    /// Transcribe speech from a WAV file with detailed results
    /// - Parameters:
    ///   - audioFilePath: Path to the WAV audio file
    ///   - language: Optional language code. If nil, auto-detect.
    /// - Returns: Transcription result with segments and metadata
    public func transcribe(audioFilePath: String, language: String? = nil) async throws -> TranscriptionResult {
        // Load audio
        let audio = try AudioProcessor.loadAudio(from: audioFilePath)
        return try transcribe(audio: audio, language: language)
    }
}

/// Errors that can occur during speech recognition
public enum SpeechRecognitionError: Error {
    case invalidAudioData
    case recognitionFailed
    case modelNotLoaded
    case modelLoadFailed
    case invalidModelPath
}

