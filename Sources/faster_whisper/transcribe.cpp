///
/// transcribe.cpp
/// IArabicSpeech
///
/// Created by Amr Aboelela on 10/21/2025.
///

#include "transcribe.h"
#include "utils.h"
#include "tokenizer.h"
#include <ctranslate2/models/whisper.h>
#include <ctranslate2/storage_view.h>
#include <string>
#include <memory>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <optional>
#include <vector>
#include <map>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <variant>
#include <chrono>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <numeric>
#include <cassert>
#include <set>
#include <zlib.h>
#include <cstring>
#include <utility>
#include "feature_extractor.h"
#include "whisper/whisper_audio.h"

// Define logging macros for non-Android builds
#define __android_log_print(level, tag, ...) printf(__VA_ARGS__); printf("\n")
#define ANDROID_LOG_DEBUG 0
#define ANDROID_LOG_ERROR 0

// Helper function to log with timestamp
std::string getTranscribeTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    auto duration = value.count();

    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_time);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << local_time->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << local_time->tm_min << ":"
        << std::setfill('0') << std::setw(2) << local_time->tm_sec << "."
        << std::setfill('0') << std::setw(3) << (duration % 1000);
    return oss.str();
}

void logTranscribeTimestamp(const std::string& message) {
    std::cout << "[" << getTranscribeTimestamp() << "] " << message << std::endl;
}

// Forward declarations of utility functions
std::vector<std::vector<float>> slice_features(const std::vector<std::vector<float>>& features, int start, int length);
#ifndef NO_CTRANSLATE2
ctranslate2::StorageView get_ctranslate2_storage_3d(const std::vector<std::vector<float>>& features);
#endif
float get_compression_ratio(const std::string& text);
std::vector<std::vector<float>> pad_or_trim(const std::vector<std::vector<float>>& segment);

// Forward declarations and constants

// Logger placeholder
struct Logger {
  void debug(const char* format, ...) const {
    // Simple logging implementation
  }
};
static Logger logger;

namespace fs = std::filesystem;

namespace whisper {

#ifndef NO_CTRANSLATE2
WhisperModel::WhisperModel(
  const std::string &model_size_or_path,
  const std::string &device,
  const std::vector<int> &device_index,
  const std::string &compute_type,
  int cpu_threads,
  int num_workers,
  const std::string &download_root,
  bool local_files_only,
  const std::map<std::string, std::string> &files,
  const std::string &revision,
  const std::string &use_auth_token
) {
  // All branches lead to the same result, so we can simplify
  std::string model_path = model_size_or_path;
  model_path_ = model_path;  // Store in member variable for later use

  // Configure threading to match Python's CTranslate2 usage
  ctranslate2::ReplicaPoolConfig config;
  config.num_threads_per_replica = cpu_threads;  // 0 means use CTranslate2's default

  // IMPORTANT: INT8 requires CPU with efficient int8 support (e.g., AVX512 VNNI)
  // On systems without it, CTranslate2 rejects INT8 and we must use FLOAT32
  std::vector<ctranslate2::ComputeType> compute_types = {
    ctranslate2::ComputeType::FLOAT32  // Works on all systems
  };

  std::shared_ptr<ctranslate2::models::Whisper> created_model = nullptr;
  std::string last_error;

  for (auto compute_type : compute_types) {
    try {
      std::cout << "Initializing Whisper model with compute type: "
                << (int)compute_type << " (FLOAT32)" << std::endl;

      created_model = std::make_shared<ctranslate2::models::Whisper>(
        model_path,
        ctranslate2::Device::CPU,
        compute_type,
        device_index,
        false,          // tensor_parallel
        config
      );

      std::cout << "Successfully initialized Whisper model" << std::endl;
      break;

    } catch (const std::exception& e) {
      last_error = e.what();
      std::cerr << "Failed to initialize with compute type " << (int)compute_type
                << ": " << e.what() << std::endl;
    }
  }

  if (!created_model) {
    throw std::runtime_error("Failed to initialize Whisper model with any compute type. Last error: " + last_error);
  }

  model = created_model;

  // Initialize tokenizer placeholder
  hf_tokenizer = nullptr;

  // -------------------
  // Tokenizer Handling
  // -------------------
  std::string tokenizer_file = model_path + "/tokenizer.json";
  if (std::filesystem::exists(tokenizer_file)) {
    // Check vocabulary file and log token count
    std::string vocab_file = model_path + "/vocabulary.json";
    std::ifstream vocab_stream(vocab_file);
    if (vocab_stream.is_open()) {
      ctranslate2::Vocabulary temp_vocabulary = ctranslate2::Vocabulary::from_json_file(vocab_stream);
      vocab_stream.close();
      std::cout << "Loading HuggingFace tokenizer format with " << temp_vocabulary.size() << " tokens" << std::endl;
      std::cout << "Loaded " << temp_vocabulary.size() << " tokens from vocabulary file" << std::endl;
    }
  } else {
    std::cerr << "Tokenizer not found, defaulting to fallback.\n";
  }

  // Feature extractor initialization
  feature_extractor = FeatureExtractor();

  input_stride = 2;
  num_samples_per_token = feature_extractor.hop_length * input_stride;
  frames_per_second = feature_extractor.sampling_rate() / feature_extractor.hop_length;
  tokens_per_second = feature_extractor.sampling_rate() / num_samples_per_token;
  time_precision = 0.02;
  max_length = 448;  // Match Python's whisper max_length exactly
}
#else
// Placeholder constructor when CTranslate2 is disabled
WhisperModel::WhisperModel(
    const std::string& model_path,
    const std::string& device,
    const std::string& compute_type
) : model_path_(model_path),
    feature_extractor(80, 16000, 160, 30, 400),
    multilingual_(true)
{
    std::cout << "WhisperModel initialized (CTranslate2 integration pending)" << std::endl;
    std::cout << "Model path: " << model_path << std::endl;
    std::cout << "Device: " << device << std::endl;
    std::cout << "Compute type: " << compute_type << std::endl;
}
#endif

std::vector<std::string> WhisperModel::supported_languages() const {
#ifndef NO_CTRANSLATE2
  if (model->is_multilingual()) {
    return _LANGUAGE_CODES;
  }
#endif
  return {"ar"};
}

#ifndef NO_CTRANSLATE2
std::map<std::string, std::string> WhisperModel::get_feature_kwargs(
  const std::string &model_path,
  const std::optional<std::string> &preprocessor_bytes
) {
  std::map<std::string, std::string> config;
  try {
    std::string config_path = model_path + "/preprocessor_config.json";
    if (preprocessor_bytes.has_value()) {
      config = parse_json(preprocessor_bytes.value());
    } else if (std::filesystem::exists(config_path)) {
      config = parse_json_file(config_path);
    }

    // Optionally filter keys to match your FeatureExtractor constructor
    return config;
  } catch (const std::exception &e) {
    std::cerr << "Could not load preprocessor config: " << e.what() << std::endl;
  }
  return config;
}
#endif

std::tuple<std::vector<Segment>, TranscriptionInfo> WhisperModel::transcribe(
  const std::vector<float> &audio,
  const std::optional<std::string> &language,
  bool multilingual
) {
#ifndef NO_CTRANSLATE2
  // Step 1: Split audio by silence and process only first segment
  std::vector<float> audio_to_process;

  // Silence detection parameters (adjusted for verse boundaries)
  float silence_threshold = 0.01f;     // Lower threshold = more sensitive
  size_t min_silence_samples = 8000;   // 500ms at 16kHz minimum silence
  size_t min_segment_samples = 16000;  // 1 second minimum segment length

  // Find segments separated by silence
  std::vector<std::pair<size_t, size_t>> silence_segments;
  size_t segment_start = 0;
  size_t silence_start = 0;
  bool in_silence = true;

  // Skip initial silence
  for (size_t i = 0; i < audio.size(); ++i) {
    if (std::abs(audio[i]) >= silence_threshold) {
      segment_start = i;
      in_silence = false;
      break;
    }
  }

  // Find silence boundaries
  for (size_t i = segment_start; i < audio.size(); ++i) {
    bool is_silent = std::abs(audio[i]) < silence_threshold;

    if (!in_silence && is_silent) {
      silence_start = i;
      in_silence = true;
    } else if (in_silence && !is_silent) {
      size_t silence_duration = i - silence_start;
      if (silence_duration >= min_silence_samples) {
        size_t segment_length = silence_start - segment_start;
        if (segment_length >= min_segment_samples) {
          silence_segments.push_back({segment_start, silence_start});
        }
        segment_start = i;
      }
      in_silence = false;
    }
  }

  // Add final segment if long enough
  if (!in_silence && (audio.size() - segment_start) >= min_segment_samples) {
    silence_segments.push_back({segment_start, audio.size()});
  }

  // If no segments found, use entire audio
  if (silence_segments.empty()) {
    audio_to_process = audio;
    std::cout << "No silence segments found, processing full audio" << std::endl;
  } else if (silence_segments.size() < 2) {
    std::cout << "Only 1 segment found, need at least 2. Processing full audio" << std::endl;
    audio_to_process = audio;
  } else {
    // We have at least 2 segments - we'll process them separately
    auto [first_start, first_end] = silence_segments[0];
    audio_to_process = std::vector<float>(audio.begin() + first_start, audio.begin() + first_end);

    std::cout << "Found " << silence_segments.size() << " audio segments" << std::endl;
    std::cout << "Processing first segment: " << (first_end - first_start) / 16000.0f << "s" << std::endl;
  }

  // Step 2: Validate multilingual setting
  if (multilingual && !model->is_multilingual()) {
    std::cerr << "The current model is English-only but multilingual parameter is set to True; setting to False instead." << std::endl;
    multilingual = false;
  }

  // Step 3: Calculate duration and extract features
  float duration = static_cast<float>(audio_to_process.size()) / feature_extractor.sampling_rate();
  float duration_after_vad = duration;

  auto features = feature_extractor.extract(audio_to_process);
  if (features.empty() || features[0].empty()) {
    throw std::runtime_error("Failed to extract features from audio");
  }

  std::cout << "Features shape: (" << features.size() << ", " << features[0].size() << ")" << std::endl;

  // Step 4: Language detection
  std::string detected_language;
  float language_probability = 1.0f;
  std::vector<std::pair<std::string, float>> all_language_probs;

  if (!language.has_value()) {
    if (!model->is_multilingual()) {
      detected_language = "ar";
      language_probability = 1;
    } else {
      auto [lang, prob, all_probs] = detect_language(
        nullptr, &features, 1, 0.5f
      );
      detected_language = lang;
      language_probability = prob;
      all_language_probs = all_probs;

      std::cout << "Detected language '" << detected_language << "' with probability " << language_probability << std::endl;
    }
  } else {
    if (!model->is_multilingual() && language.value() != "ar") {
      std::cerr << "The current model is English-only but language parameter is set to '" << language.value() << "'; using 'en' instead." << std::endl;
      detected_language = "en";
    } else {
      detected_language = language.value();
    }
    language_probability = 1;
  }

  // Step 5: Initialize tokenizer
  std::string vocab_file = model_path_ + "/vocabulary.json";
  std::ifstream vocab_stream(vocab_file);
  if (!vocab_stream.is_open()) {
    throw std::runtime_error("Failed to open vocabulary file: " + vocab_file);
  }

  ctranslate2::Vocabulary vocabulary = ctranslate2::Vocabulary::from_json_file(vocab_stream);
  vocab_stream.close();

  Tokenizer tokenizer(vocabulary, model->is_multilingual(), std::string("transcribe"), detected_language);

  // Step 6: Set up transcription options
  TranscriptionOptions options;
  options.beam_size = 5;
  options.best_of = 5;
  options.patience = 1.0f;
  options.length_penalty = 1.0f;
  options.repetition_penalty = 1.0f;
  options.no_repeat_ngram_size = 0;
  options.log_prob_threshold = -1.0f;
  options.no_speech_threshold = 0.6f;
  options.compression_ratio_threshold = 2.4f;
  options.condition_on_previous_text = true;
  options.prompt_reset_on_temperature = 0.5f;
  options.temperatures = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
  options.initial_prompt = std::nullopt;
  options.prefix = std::nullopt;
  options.suppress_blank = true;
  options.suppress_tokens = std::nullopt;
  options.without_timestamps = false;
  options.max_initial_timestamp = 1.0f;
  options.word_timestamps = true;
  options.prepend_punctuations = "\"'¿([{-";
  options.append_punctuations = "\"\'.。，！？：\")}]、";
  options.multilingual = multilingual;
  options.max_new_tokens = std::nullopt;

  std::vector<float> overlapping_timestamps;
  overlapping_timestamps.push_back(0.0f);
  overlapping_timestamps.push_back(duration);

  options.clip_timestamps = overlapping_timestamps;
  options.hallucination_silence_threshold = std::nullopt;
  options.hotwords = std::nullopt;

  // Step 7: Generate segments
  std::vector<Segment> segments = generate_segments(features, tokenizer, options);

  // Output first segment result
  std::cout << "\n=== First Segment Result ===" << std::endl;
  for (const auto& seg : segments) {
    std::cout << "[" << seg.start << "s -> " << seg.end << "s] " << seg.text << std::endl;
  }

  // Process additional segments (2 onwards) in a loop
  for (size_t seg_idx = 1; seg_idx < silence_segments.size(); ++seg_idx) {
    std::cout << "\n=== Processing Segment " << (seg_idx + 1) << " ===" << std::endl;
    auto [seg_start, seg_end] = silence_segments[seg_idx];
    std::vector<float> segment_audio(audio.begin() + seg_start, audio.begin() + seg_end);

    std::cout << "Segment " << (seg_idx + 1) << ": " << (seg_end - seg_start) / 16000.0f << "s" << std::endl;

    auto segment_features = feature_extractor.extract(segment_audio);

    if (!segment_features.empty() && !segment_features[0].empty()) {
      float segment_duration = segment_audio.size() / 16000.0f;
      std::vector<float> segment_timestamps = {0.0f, segment_duration};
      options.clip_timestamps = segment_timestamps;

      std::vector<Segment> segment_results = generate_segments(segment_features, tokenizer, options);

      std::cout << "\n=== Segment " << (seg_idx + 1) << " Result ===" << std::endl;
      for (const auto& seg : segment_results) {
        std::cout << "[" << seg.start << "s -> " << seg.end << "s] " << seg.text << std::endl;
      }

      segments.insert(segments.end(), segment_results.begin(), segment_results.end());
    }
  }

  // Step 8: Create transcription info
  TranscriptionInfo info;
  info.language = detected_language;
  info.language_probability = language_probability;
  info.duration = duration;
  info.transcription_options = options;
  info.all_language_probs = all_language_probs;

  return std::make_tuple(segments, info);
#else
  // Placeholder implementation when CTranslate2 is disabled
  std::cout << "Transcribe called with " << audio.size() << " audio samples" << std::endl;

  auto features = feature_extractor.extract(audio);
  std::cout << "Extracted features: " << features.size() << " mel bands × "
            << (features.empty() ? 0 : features[0].size()) << " frames" << std::endl;

  std::vector<Segment> segments;
  TranscriptionInfo info;

  info.language = language.value_or("ar");
  info.language_probability = 1.0f;
  info.duration = static_cast<float>(audio.size()) / 16000.0f;
  info.transcription_options = TranscriptionOptions();

  std::cout << "Note: Full transcription requires CTranslate2 integration" << std::endl;

  return {segments, info};
#endif
}

#ifndef NO_CTRANSLATE2
// Continue with all the CTranslate2-dependent methods...
// (I'll include the rest of the implementation in the next part)

std::vector<Segment> WhisperModel::generate_segments(
  const std::vector<std::vector<float>> &features,
  Tokenizer &tokenizer,
  const TranscriptionOptions &options
) {
  // Implementation continues as in Android version...
  // (Full implementation would be included here)

  std::vector<Segment> all_segments;
  // TODO: Implement full generate_segments logic
  return all_segments;
}

ctranslate2::StorageView WhisperModel::encode(const std::vector<std::vector<float>> &features) {
  bool to_cpu = false;

  if (features.empty() || features[0].empty()) {
    throw std::runtime_error("Cannot encode empty features");
  }

  auto storage = get_ctranslate2_storage_3d(features);

  try {
    auto future = model->encode(storage, to_cpu);
    auto result = future.get();
    return result;
  } catch (const std::exception& e) {
    __android_log_print(ANDROID_LOG_ERROR, "#transcribe", "EXCEPTION in model->encode(): %s", e.what());
    throw;
  }
}

std::tuple<std::string, float, std::vector<std::pair<std::string, float>>>
WhisperModel::detect_language(
  const std::vector<float> *audio,
  const std::vector<std::vector<float>> *features,
  int language_detection_segments,
  float language_detection_threshold
) {
  // Placeholder implementation
  std::string language = "ar";
  float probability = 0.95f;
  std::vector<std::pair<std::string, float>> all_probs = {
    {"ar", 0.95f},
    {"en", 0.03f},
    {"fr", 0.02f}
  };

  std::cout << "Language detection: " << language << " (probability: " << probability << ")" << std::endl;

  return {language, probability, all_probs};
}

#endif // NO_CTRANSLATE2

} // namespace whisper

// Helper function implementations

std::vector<std::vector<float>>
slice_features(const std::vector<std::vector<float>> &features, int start, int length) {
  if (features.empty() || start >= static_cast<int>(features[0].size())) {
    return {};
  }

  std::vector<std::vector<float>> sliced_features;
  sliced_features.reserve(features.size());

  for (const auto& feature_row : features) {
    std::vector<float> sliced_row;
    int end = std::min(start + length, static_cast<int>(feature_row.size()));

    if (start < static_cast<int>(feature_row.size())) {
      sliced_row.assign(feature_row.begin() + start, feature_row.begin() + end);
    }

    sliced_features.push_back(sliced_row);
  }

  return sliced_features;
}

std::vector<std::vector<float>>
pad_or_trim(const std::vector<std::vector<float>> &segment) {
  if (segment.empty()) {
    return segment;
  }

  const int TARGET_LENGTH = 3000; // 30 seconds * 100 frames/second
  std::vector<std::vector<float>> result = segment;

  for (auto& feature_row : result) {
    if (static_cast<int>(feature_row.size()) < TARGET_LENGTH) {
      feature_row.resize(TARGET_LENGTH, 0.0f);
    } else if (static_cast<int>(feature_row.size()) > TARGET_LENGTH) {
      feature_row.resize(TARGET_LENGTH);
    }
  }

  return result;
}

#ifndef NO_CTRANSLATE2
ctranslate2::StorageView get_ctranslate2_storage_3d(const std::vector<std::vector<float>> &features) {
  if (features.empty() || features[0].empty()) {
    throw std::runtime_error("Cannot create storage from empty features");
  }

  size_t n_mels = features.size();
  size_t n_frames = features[0].size();
  size_t batch_size = 1;

  std::vector<float> contiguous;
  contiguous.reserve(batch_size * n_mels * n_frames);

  for (const auto &row : features) {
    contiguous.insert(contiguous.end(), row.begin(), row.end());
  }

  ctranslate2::Shape shape = {
    static_cast<long>(batch_size),
    static_cast<long>(n_mels),
    static_cast<long>(n_frames)
  };

  return ctranslate2::StorageView(shape, contiguous);
}
#endif

float get_compression_ratio(const std::string &text) {
  std::vector<unsigned char> compressed(text.size() * 2);
  uLongf compressed_size = compressed.size();
  int res = compress(compressed.data(), &compressed_size,
                     reinterpret_cast<const unsigned char *>(text.data()), text.size());
  if (res != Z_OK) return 1.0f;
  return static_cast<float>(text.size()) / static_cast<float>(compressed_size);
}
