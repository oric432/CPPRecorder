#include "AudioFileWriter.h"

// Save the recorded data to a WAV file
void AudioFileWriter::saveToWav(const std::string &filename, const std::vector<float> &audioData, int bitsPerSample, int sampleRate, int numChannels)
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
    int dataSize = audioData.size() * sizeof(int16_t);

    // Write the WAV header
    writeWavHeader(outFile, sampleRate, numChannels, bitsPerSample, dataSize);

    // Write audio data
    for (float sample : audioData)
    {
        // Convert float sample [-1.0, 1.0] to 16-bit PCM [-32768, 32767]
        int16_t pcmSample = static_cast<int16_t>(sample * INT16_MAX);
        outFile.write(reinterpret_cast<const char *>(&pcmSample), sizeof(pcmSample));
    }

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
