///
/// whisper_model_caller.cpp
/// Standalone whisper model caller for integration testing
///
/// Usage: whisper_model_caller <audio_file> <model_path> [language]
///

#include "transcribe.h"
#include "audio.h"
#include <iostream>
#include <string>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <audio_file> <model_path> [language]" << std::endl;
        return 1;
    }

    std::string audioFile = argv[1];
    std::string modelPath = argv[2];
    std::string language = (argc >= 4) ? argv[3] : "ar";

    try {
        // Load audio
        std::vector<float> audio = Audio::decode_audio(audioFile, 16000);

        // Load model
        WhisperModel model(
            modelPath,
            "cpu",
            {0},
            "float32",
            0,
            1
        );

        // Transcribe
        auto [segments, info] = model.transcribe(audio, language, true);

        // Print results
        std::cout << "\n=== Transcription Results ===" << std::endl;
        std::cout << "Language: " << info.language << " (confidence: "
                  << info.language_probability << ")" << std::endl;
        std::cout << "Duration: " << info.duration << "s" << std::endl;
        std::cout << "Segments: " << segments.size() << std::endl;

        std::cout << "\n=== Full Transcription ===" << std::endl;
        for (const auto& segment : segments) {
            std::cout << segment.text;
        }
        std::cout << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
