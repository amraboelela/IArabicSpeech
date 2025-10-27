///
/// test_whisper.cpp
/// Whisper Integration Tests
///
/// Tests the full Whisper transcription pipeline by calling the whisper_model_caller
/// and comparing results with expected outputs.
///

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

class WhisperTester {
private:
    std::string modelPath;

public:
    WhisperTester() {
        modelPath = findWhisperModelPath();
    }

    std::string findWhisperModelPath() {
        std::vector<std::string> possiblePaths = {
            "../../../Sources/faster_whisper/model/whisper_ct2",
            "../../Sources/faster_whisper/model/whisper_ct2"
        };

        for (const auto& path : possiblePaths) {
            if (fs::exists(path)) {
                std::cout << "Found model at: " << path << std::endl;
                return fs::absolute(path).string();
            }
        }

        throw std::runtime_error("Could not find whisper model in any expected location");
    }

    bool runTest(const std::string& audioFile, const std::string& expectedText,
                 const std::string& language = "ar") {
        std::cout << "\n=== Testing: " << audioFile << " ===" << std::endl;

        // Check if audio file exists
        std::string audioPath = "../assets/" + audioFile;
        if (!fs::exists(audioPath)) {
            std::cerr << "Audio file not found: " << audioPath << std::endl;
            return false;
        }

        // Build command
        std::string command = "./whisper_model_caller " +
                            fs::absolute(audioPath).string() + " " +
                            modelPath + " " + language;

        std::cout << "Running: " << command << std::endl;

        // Execute command and capture output
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run whisper_model_caller" << std::endl;
            return false;
        }

        std::string result;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
            std::cout << buffer;
        }

        int returnCode = pclose(pipe);

        if (returnCode != 0) {
            std::cerr << "whisper_model_caller failed with code: " << returnCode << std::endl;
            return false;
        }

        // Simple check: see if result contains some expected text
        if (!expectedText.empty() && result.find(expectedText) == std::string::npos) {
            std::cerr << "Expected text not found in output" << std::endl;
            std::cerr << "Expected substring: " << expectedText << std::endl;
            std::cerr << "Got output: " << result << std::endl;
            return false;
        }

        std::cout << "✓ Test passed!" << std::endl;
        return true;
    }

    void runAllTests() {
        std::cout << "=== Whisper Integration Tests ===" << std::endl;
        std::cout << "Model path: " << modelPath << std::endl;

        int passed = 0;
        int total = 0;

        // Test 1: Al-Fatiha (001.wav)
        total++;
        if (runTest("001.wav", "الله", "ar")) {
            passed++;
        }

        // Test 2: 002-01.wav (longer Arabic)
        total++;
        if (runTest("002-01.wav", "الله", "ar")) {
            passed++;
        }

        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << "/" << total << std::endl;

        if (passed == total) {
            std::cout << "✅ All tests passed!" << std::endl;
        } else {
            std::cout << "❌ Some tests failed" << std::endl;
            exit(1);
        }
    }
};

int main() {
    try {
        WhisperTester tester;
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
