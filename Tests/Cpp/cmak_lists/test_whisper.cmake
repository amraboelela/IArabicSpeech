cmake_minimum_required(VERSION 3.10)
project(WhisperIntegrationTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy source files to build directory
configure_file(${CMAKE_SOURCE_DIR}/../test_whisper.cpp ${CMAKE_SOURCE_DIR}/test_whisper.cpp COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../whisper_model_caller.cpp ${CMAKE_SOURCE_DIR}/whisper_model_caller.cpp COPYONLY)

# Copy assets directory if it exists
if(EXISTS ${CMAKE_SOURCE_DIR}/../assets)
    file(COPY ${CMAKE_SOURCE_DIR}/../assets DESTINATION ${CMAKE_SOURCE_DIR})
endif()

# Add main test executable
add_executable(test_whisper test_whisper.cpp)

# Set paths - get absolute path to project root
get_filename_component(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE)
set(FASTER_WHISPER_DIR "${PROJECT_ROOT}/Sources/faster_whisper")
set(CTRANSLATE2_FRAMEWORK_PATH "${PROJECT_ROOT}/Frameworks/CTranslate2.xcframework/macos-arm64/CTranslate2.framework")

message(STATUS "Building whisper_model_caller with CTranslate2")
message(STATUS "Project root: ${PROJECT_ROOT}")
message(STATUS "Framework path: ${CTRANSLATE2_FRAMEWORK_PATH}")

# Add whisper model caller with C++ implementation
add_executable(whisper_model_caller whisper_model_caller.cpp
    ${FASTER_WHISPER_DIR}/transcribe.cpp
    ${FASTER_WHISPER_DIR}/audio.cpp
    ${FASTER_WHISPER_DIR}/feature_extractor.cpp
    ${FASTER_WHISPER_DIR}/tokenizer.cpp
    ${FASTER_WHISPER_DIR}/utils.cpp
    ${FASTER_WHISPER_DIR}/whisper/whisper_tokenizer.cpp
    ${FASTER_WHISPER_DIR}/whisper/whisper_audio.cpp
)

# Add include directories
target_include_directories(whisper_model_caller PRIVATE
    ${FASTER_WHISPER_DIR}/include
    ${FASTER_WHISPER_DIR}/headers
    ${FASTER_WHISPER_DIR}/whisper
    ${CTRANSLATE2_FRAMEWORK_PATH}/Headers
)

# Find CTranslate2 from the bundled xcframework
set(CTRANSLATE2_FRAMEWORK "${CTRANSLATE2_FRAMEWORK_PATH}/CTranslate2")
find_library(ZLIB_LIB z)

# On macOS, find Accelerate framework for BLAS support
if(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
endif()

if(EXISTS ${CTRANSLATE2_FRAMEWORK} AND ZLIB_LIB)
    target_link_libraries(whisper_model_caller ${CTRANSLATE2_FRAMEWORK} ${ZLIB_LIB})

    # Add Accelerate framework on macOS
    if(APPLE AND ACCELERATE_FRAMEWORK)
        target_link_libraries(whisper_model_caller ${ACCELERATE_FRAMEWORK})
        message(STATUS "Linked Accelerate framework: ${ACCELERATE_FRAMEWORK}")
    endif()

    message(STATUS "Linked CTranslate2 framework: ${CTRANSLATE2_FRAMEWORK}")
    message(STATUS "Linked zlib: ${ZLIB_LIB}")
else()
    if(NOT EXISTS ${CTRANSLATE2_FRAMEWORK})
        message(WARNING "CTranslate2 framework not found: ${CTRANSLATE2_FRAMEWORK}")
    endif()
    if(NOT ZLIB_LIB)
        message(WARNING "zlib not found")
    endif()
    message(WARNING "Build may fail without required libraries")
endif()

# Link filesystem library based on platform and compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(test_whisper stdc++fs)
    target_link_libraries(whisper_model_caller stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(test_whisper c++fs)
    target_link_libraries(whisper_model_caller c++fs)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_whisper PRIVATE -Wall -Wextra)
    target_compile_options(whisper_model_caller PRIVATE -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(test_whisper PRIVATE -Wall -Wextra)
    target_compile_options(whisper_model_caller PRIVATE -Wall -Wextra)
endif()

# Set output directory
set_target_properties(test_whisper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(whisper_model_caller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
