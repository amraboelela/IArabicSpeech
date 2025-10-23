///
/// transcribe.h
/// IArabicSpeech
///
/// Created by Amr Aboelela on 10/21/2025.
///

#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H

#include "feature_extractor.h"
#include "tokenizer.h"
#include "whisper/whisper_tokenizer.h"
#include <ctranslate2/models/whisper.h>
#include <ctranslate2/storage_view.h>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <memory>

namespace whisper {

/// Word-level transcription result
struct Word {
    float start;
    float end;
    std::string word;
    float probability;
};

/// Segment-level transcription result
struct Segment {
    int id;
    int seek;
    float start;
    float end;
    std::string text;
    std::vector<int> tokens;
    float avg_logprob;
    float compression_ratio;
    float no_speech_prob;
    std::optional<std::vector<Word>> words;
    std::optional<float> temperature;
};

/// Transcription options
struct TranscriptionOptions {
    int beam_size = 5;
    int best_of = 5;
    float patience = 1.0f;
    float length_penalty = 1.0f;
    float repetition_penalty = 1.0f;
    int no_repeat_ngram_size = 0;

    std::optional<float> log_prob_threshold;
    std::optional<float> no_speech_threshold;
    std::optional<float> compression_ratio_threshold;

    bool condition_on_previous_text = true;
    float prompt_reset_on_temperature = 0.5f;
    std::vector<float> temperatures = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};

    std::optional<std::string> initial_prompt;
    std::optional<std::string> prefix;
    bool suppress_blank = true;
    std::optional<std::vector<int>> suppress_tokens;
    bool without_timestamps = false;
    float max_initial_timestamp = 1.0f;
    bool word_timestamps = true;
    std::string prepend_punctuations = "\"'¿([{-";
    std::string append_punctuations = "\"\'.。，！？：\")}]、";
    bool multilingual = false;
    std::optional<int> max_new_tokens;

    // Additional fields used in transcribe.cpp
    std::vector<float> clip_timestamps;
    std::optional<float> hallucination_silence_threshold;
    std::optional<std::string> hotwords;
};

/// Transcription information
struct TranscriptionInfo {
    std::string language;
    float language_probability;
    float duration;
    std::optional<std::vector<std::pair<std::string, float>>> all_language_probs;
    TranscriptionOptions transcription_options;
};

/// Whisper Model for Arabic speech recognition
class WhisperModel {
public:
    /// Constructor with full CTranslate2 integration
    WhisperModel(
        const std::string& model_size_or_path,
        const std::string& device,
        const std::vector<int>& device_index,
        const std::string& compute_type,
        int cpu_threads,
        int num_workers,
        const std::string& download_root,
        bool local_files_only,
        const std::map<std::string, std::string>& files,
        const std::string& revision,
        const std::string& use_auth_token
    );

    /// Transcribe audio to text
    /// @param audio Audio samples at 16kHz
    /// @param language Optional language code (default: detect automatically)
    /// @param multilingual Whether to use multilingual model
    /// @return Tuple of (segments, transcription_info)
    std::tuple<std::vector<Segment>, TranscriptionInfo> transcribe(
        const std::vector<float>& audio,
        const std::optional<std::string>& language = std::nullopt,
        bool multilingual = false
    );

    /// Get supported languages
    std::vector<std::string> supported_languages() const;

    /// Detect language from audio
    std::tuple<std::string, float, std::vector<std::pair<std::string, float>>> detect_language(
        const std::vector<float>* audio = nullptr,
        const std::vector<std::vector<float>>* features = nullptr,
        int language_detection_segments = 1,
        float language_detection_threshold = 0.5f
    );

    /// Generate segments from features
    std::vector<Segment> generate_segments(
        const std::vector<std::vector<float>>& features,
        Tokenizer& tokenizer,
        const TranscriptionOptions& options
    );

    /// Encode features using model
    ctranslate2::StorageView encode(const std::vector<std::vector<float>>& features);

    /// Get feature extraction kwargs
    static std::map<std::string, std::string> get_feature_kwargs(
        const std::string& model_path,
        const std::optional<std::string>& preprocessor_bytes = std::nullopt
    );

private:
    std::string model_path_;
    std::shared_ptr<ctranslate2::models::Whisper> model;
    void* hf_tokenizer;  // Placeholder for tokenizer
    FeatureExtractor feature_extractor;
    bool multilingual_;
    int input_stride;
    int num_samples_per_token;
    float frames_per_second;
    float tokens_per_second;
    float time_precision;
    int max_length;
};

} // namespace whisper

#endif // TRANSCRIBE_H
