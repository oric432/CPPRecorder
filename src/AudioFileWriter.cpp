#include "AudioFileWriter.h"

// Save the recorded data to a WAV file
void AudioFileWriter::saveToWav(const std::string &filename, const int16_t *audioData, size_t dataSize, int bitsPerSample, int sampleRate, int numChannels)
{
    // Open file in binary mode
    std::ofstream outFile(filename, std::ios::binary);

    if (!outFile)
    {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    // WAV file parameters
    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    int blockAlign = numChannels * bitsPerSample / 8;

    // Write the WAV header
    writeWavHeader(outFile, sampleRate, numChannels, bitsPerSample, dataSize);

    // Write audio data
    outFile.write(reinterpret_cast<const char *>(audioData), dataSize);

    outFile.close();
    std::cout << "Audio data saved to " << filename << std::endl;
}

// Write the WAV file header
void AudioFileWriter::writeWavHeader(std::ofstream &outFile, int sampleRate, int numChannels, int bitsPerSample, int dataSize)
{
    int chunkSize = 36 + dataSize;
    int subchunk1Size = 16;
    int audioFormat = 1; // PCM format
    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    int blockAlign = numChannels * bitsPerSample / 8;

    // Write RIFF header
    outFile.write("RIFF", 4);
    outFile.write(reinterpret_cast<const char *>(&chunkSize), 4);
    outFile.write("WAVE", 4);

    // Write fmt subchunk
    outFile.write("fmt ", 4);
    outFile.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
    outFile.write(reinterpret_cast<const char *>(&audioFormat), 2);
    outFile.write(reinterpret_cast<const char *>(&numChannels), 2);
    outFile.write(reinterpret_cast<const char *>(&sampleRate), 4);
    outFile.write(reinterpret_cast<const char *>(&byteRate), 4);
    outFile.write(reinterpret_cast<const char *>(&blockAlign), 2);
    outFile.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

    // Write data subchunk
    outFile.write("data", 4);
    outFile.write(reinterpret_cast<const char *>(&dataSize), 4);
}
