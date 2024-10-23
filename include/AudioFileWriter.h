#ifndef AUDIO_FILE_WRITER_H
#define AUDIO_FILE_WRITER_H

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

class AudioFileWriter
{
public:
    static const size_t BUFFER_SIZE = 4096;

public:
    // Save a single audio file
    void saveToWav(const std::string &filename, const int16_t *audioData, size_t dataSize, int bitsPerSample, int sampleRate, int numChannels);

    // Merge WAV files using a prefix and a count
    void mergeWavFiles(const std::string &outputFilename, const std::string &prefix, size_t count);

private:
    // Write the WAV file header
    void writeWavHeader(std::ofstream &outFile, int sampleRate, int numChannels, int bitsPerSample, int dataSize);

    // Append audio data from one file to the output stream
    void appendAudioData(std::ifstream &inputFile, std::ofstream &outputFile);

    // Update the WAV header with the correct file size
    void updateWavHeader(std::ofstream &outFile, std::uint32_t totalDataSize);

    // Construct filename from prefix and index
    std::string constructFilename(const std::string &prefix, size_t index);
};

#endif // AUDIO_FILE_WRITER_H