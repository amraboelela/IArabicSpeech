///
/// IArabicSpeechBridge.cpp
/// IArabicSpeech
///
/// Created by Amr Aboelela on 10/21/2025.
///

#include "IArabicSpeech-Bridging.h"
#include "whisper/whisper_audio.h"
#include "feature_extractor.h"
#include "transcribe.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C" {

FloatArray whisper_load_audio(const char* filename) {
    FloatArray result = {nullptr, 0};

    if (!filename) {
        return result;
    }

    // Load audio using whisper audio processor
    std::vector<float> audio = whisper::AudioProcessor::load_audio(filename);

    if (audio.empty()) {
        return result;
    }

    // Allocate C array and copy data
    result.length = audio.size();
    result.data = static_cast<float*>(malloc(result.length * sizeof(float)));
    if (result.data) {
        std::memcpy(result.data, audio.data(), result.length * sizeof(float));
    } else {
        result.length = 0;
    }

    return result;
}

FloatMatrix whisper_extract_mel_spectrogram(const float* audio, unsigned long length) {
    FloatMatrix result = {nullptr, 0, 0};

    if (!audio || length == 0) {
        return result;
    }

    // Convert to std::vector
    std::vector<float> audio_vec(audio, audio + length);

    // Create feature extractor
    FeatureExtractor extractor(80, 16000, 160, 30, 400);

    // Extract mel spectrogram
    Matrix mel_spec = extractor.compute_mel_spectrogram(audio_vec, 160);

    if (mel_spec.empty()) {
        return result;
    }

    // Allocate C 2D array and copy data
    result.rows = mel_spec.size();
    result.cols = mel_spec[0].size();
    result.data = static_cast<float**>(malloc(result.rows * sizeof(float*)));

    if (result.data) {
        for (unsigned long i = 0; i < result.rows; ++i) {
            result.data[i] = static_cast<float*>(malloc(result.cols * sizeof(float)));
            if (result.data[i]) {
                std::memcpy(result.data[i], mel_spec[i].data(), result.cols * sizeof(float));
            } else {
                // Cleanup on failure
                for (unsigned long j = 0; j < i; ++j) {
                    free(result.data[j]);
                }
                free(result.data);
                result.data = nullptr;
                result.rows = 0;
                result.cols = 0;
                return result;
            }
        }
    } else {
        result.rows = 0;
        result.cols = 0;
    }

    return result;
}

void whisper_free_float_array(FloatArray array) {
    if (array.data) {
        free(array.data);
    }
}

void whisper_free_float_matrix(FloatMatrix matrix) {
    if (matrix.data) {
        for (unsigned long i = 0; i < matrix.rows; ++i) {
            if (matrix.data[i]) {
                free(matrix.data[i]);
            }
        }
        free(matrix.data);
    }
}

// Model management and transcription functions

WhisperModelHandle whisper_create_model(const char* model_path) {
    if (!model_path) {
        return nullptr;
    }

    try {
        // Create WhisperModel with full CTranslate2 parameters
        auto* model = new whisper::WhisperModel(
            model_path,           // model_size_or_path
            "cpu",                // device
            {},                   // device_index
            "float32",            // compute_type
            0,                    // cpu_threads (0 = auto)
            1,                    // num_workers
            "",                   // download_root
            false,                // local_files_only
            {},                   // files
            "",                   // revision
            ""                    // use_auth_token
        );
        return static_cast<WhisperModelHandle>(model);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create Whisper model: " << e.what() << std::endl;
        return nullptr;
    }
}

void whisper_destroy_model(WhisperModelHandle model) {
    if (model) {
        delete static_cast<whisper::WhisperModel*>(model);
    }
}

TranscriptionResult whisper_transcribe(
    WhisperModelHandle model,
    const float* audio,
    unsigned long audio_length,
    const char* language
) {
    TranscriptionResult result = {nullptr, 0, nullptr, 0.0f, 0.0f};

    if (!model || !audio || audio_length == 0) {
        return result;
    }

    try {
        auto* whisper_model = static_cast<whisper::WhisperModel*>(model);

        // Convert audio to std::vector
        std::vector<float> audio_vec(audio, audio + audio_length);

        // Transcribe
        std::optional<std::string> lang = language ? std::optional<std::string>(language) : std::nullopt;
        auto [segments, info] = whisper_model->transcribe(audio_vec, lang, true);

        // Allocate and copy segments
        result.segment_count = segments.size();
        if (result.segment_count > 0) {
            result.segments = static_cast<TranscriptionSegment*>(
                malloc(result.segment_count * sizeof(TranscriptionSegment))
            );

            for (size_t i = 0; i < segments.size(); ++i) {
                const auto& seg = segments[i];

                // Allocate and copy text
                result.segments[i].text = static_cast<char*>(malloc(seg.text.length() + 1));
                std::strcpy(result.segments[i].text, seg.text.c_str());

                result.segments[i].start = seg.start;
                result.segments[i].end = seg.end;
            }
        }

        // Allocate and copy language
        result.language = static_cast<char*>(malloc(info.language.length() + 1));
        std::strcpy(result.language, info.language.c_str());

        result.language_probability = info.language_probability;
        result.duration = info.duration;

    } catch (const std::exception& e) {
        std::cerr << "Transcription failed: " << e.what() << std::endl;
    }

    return result;
}

void whisper_free_transcription_result(TranscriptionResult result) {
    if (result.segments) {
        for (unsigned long i = 0; i < result.segment_count; ++i) {
            if (result.segments[i].text) {
                free(result.segments[i].text);
            }
        }
        free(result.segments);
    }
    if (result.language) {
        free(result.language);
    }
}

} // extern "C"
