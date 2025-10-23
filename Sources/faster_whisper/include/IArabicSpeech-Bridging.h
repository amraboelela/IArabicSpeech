///
/// IArabicSpeech-Bridging.h
/// IArabicSpeech
///
/// Created by Amr Aboelela on 10/21/2025.
///

#ifndef IArabicSpeech_Bridging_h
#define IArabicSpeech_Bridging_h

#ifdef __cplusplus
extern "C" {
#endif

// C API for whisper audio processing
typedef struct {
    float* data;
    unsigned long length;
} FloatArray;

typedef struct {
    float** data;
    unsigned long rows;
    unsigned long cols;
} FloatMatrix;

// Opaque pointer to WhisperModel (C++ class)
typedef void* WhisperModelHandle;

// Transcription result structure
typedef struct {
    char* text;              // Transcribed text
    float start;             // Start time in seconds
    float end;               // End time in seconds
} TranscriptionSegment;

typedef struct {
    TranscriptionSegment* segments;
    unsigned long segment_count;
    char* language;
    float language_probability;
    float duration;
} TranscriptionResult;

// Audio processing functions
FloatArray whisper_load_audio(const char* filename);
FloatMatrix whisper_extract_mel_spectrogram(const float* audio, unsigned long length);

// Model management functions
WhisperModelHandle whisper_create_model(const char* model_path);
void whisper_destroy_model(WhisperModelHandle model);
TranscriptionResult whisper_transcribe(
    WhisperModelHandle model,
    const float* audio,
    unsigned long audio_length,
    const char* language  // NULL for auto-detect
);

// Memory cleanup functions
void whisper_free_float_array(FloatArray array);
void whisper_free_float_matrix(FloatMatrix matrix);
void whisper_free_transcription_result(TranscriptionResult result);

#ifdef __cplusplus
}
#endif

#endif /* IArabicSpeech_Bridging_h */
