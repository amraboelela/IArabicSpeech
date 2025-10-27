///
/// tokenizer.h
/// IArabicSpeech
///
/// Created by Amr Aboelela on 10/21/2025.
///

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <tuple>
#include <memory>
#ifndef NO_CTRANSLATE2
#include <ctranslate2/vocabulary.h>
#endif
#include "whisper_tokenizer.h"  // Include complete definition of TokenizerWrapper

// Forward declaration of the mock tokenizers::Tokenizer class
namespace tokenizers {
  class Tokenizer;
}

// A C++ equivalent of the `_TASKS` and `_LANGUAGE_CODES` constants.
extern const std::unordered_set<std::string> _TASKS;
extern const std::vector<std::string> _LANGUAGE_CODES;

class Tokenizer {
public:
  // C++ equivalent of the Python constructor.
  Tokenizer(
    tokenizers::Tokenizer* tokenizer,
    bool multilingual,
    std::optional<std::string> task = std::nullopt,
    std::optional<std::string> language = std::nullopt,
    std::optional<std::string> vocab_path = std::nullopt
  );

#ifndef NO_CTRANSLATE2
  // Constructor with CTranslate2 vocabulary
  Tokenizer(
    const ctranslate2::Vocabulary& vocabulary,
    bool multilingual,
    std::optional<std::string> task = std::nullopt,
    std::optional<std::string> language = std::nullopt
  );
#endif

  // C++ equivalent of the @cached_property methods.
  int get_transcribe();
  int get_translate();
  int get_sot();
  int get_sot_lm();
  int get_sot_prev();
  int get_eot();
  int get_no_timestamps();
  std::vector<int> get_non_speech_tokens();

  // C++ equivalent of the properties.
  int get_timestamp_begin();
  std::vector<int> get_sot_sequence();

  // C++ equivalent of the Python methods.
  std::vector<int> encode(const std::string& text);
  std::string decode(const std::vector<int>& tokens);
  std::string decode_with_timestamps(const std::vector<int>& tokens);

  // C++ equivalent of split_to_word_tokens().
  std::pair<std::vector<std::string>, std::vector<std::vector<int>>>
  split_to_word_tokens(const std::vector<int>& tokens);

private:
  tokenizers::Tokenizer* _tokenizer;  // Unused - kept for potential future use
  bool _multilingual;  // Unused - kept for potential future use
  std::optional<int> _task;
  std::optional<int> _language;
  std::string _language_code;

  // Whisper tokenizer wrapper for enhanced functionality
  std::unique_ptr<whisper::TokenizerWrapper> whisper_wrapper_;

  // Optional members to cache the result of the `cached_property` methods.
  std::optional<int> _transcribe;
  std::optional<int> _translate;
  std::optional<int> _sot;
  std::optional<int> _sot_lm;
  std::optional<int> _sot_prev;
  std::optional<int> _eot;
  std::optional<int> _no_timestamps;
  std::optional<std::vector<int>> _non_speech_tokens;

  // C++ equivalent of the private helper methods.
  std::pair<std::vector<std::string>, std::vector<std::vector<int>>>
  split_tokens_on_unicode(const std::vector<int>& tokens);

  std::pair<std::vector<std::string>, std::vector<std::vector<int>>>
  split_tokens_on_spaces(const std::vector<int>& tokens);
};

#endif // TOKENIZER_H
