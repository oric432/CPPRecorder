#include "AudioRecorder.h"
#include "PTTManager.h"
#include "AudioFileWriter.h"
#include <iostream>
#include <csignal>

// Global flag for graceful shutdown (Ctrl + C)
std::atomic<bool> keepRunning(true);

void signalHandler(int signum)
{
    std::cout << "\nInterrupt signal (" << signum << ") received. Exiting..." << std::endl;
    keepRunning = false;
}

int main()
{
    // Register signal handler for Ctrl + C
    signal(SIGINT, signalHandler);

    try
    {
        AudioRecorder recorder(8000, 256, 1, 10); // 8kHz, 256 frames per buffer, mono, 10 seconds
        PTTManager pttManager(recorder);

        // Start recording
        if (!recorder.startRecording())
        {
            std::cerr << "Failed to start recording." << std::endl;
            return 1;
        }

        // Start monitoring PTT key
        pttManager.startMonitoring();

        // Main loop
        while (keepRunning && recorder.isRecording())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Stop monitoring and recording
        pttManager.stopMonitoring();
        recorder.stopRecording();

        // Get recording data
        const std::vector<float> &audioData = recorder.getRecordedData();
        std::cout << "Recorded " << audioData.size() << " samples of audio data." << std::endl;

        // Save recording data as a WAV file
        AudioFileWriter fileWriter;
        fileWriter.saveToWav("output.wav", audioData, 16, 8000, 1); // 16-bit PCM, 8kHz sample rate, 1 channel (mono)
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
