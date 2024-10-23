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
    static constexpr size_t BUFFER_SIZE = 4096;
    static constexpr int WAV_HEADER_SIZE = 44;

public:
    void saveToWav(const std::string &filename, const int16_t *audioData, size_t dataSize, int bitsPerSample, int sampleRate, int numChannels);
    void mergeWavFiles(const std::string &outputFilename, const std::string &prefix, size_t count);

private:
    void writeWavHeader(std::ofstream &outFile, int sampleRate, int numChannels, int bitsPerSample, int dataSize);
    void appendAudioData(std::ifstream &inputFile, std::ofstream &outputFile);
    void updateWavHeader(std::ofstream &outFile, std::uint32_t totalDataSize);
    std::string constructFilename(const std::string &prefix, size_t index);
};

#endif // AUDIO_FILE_WRITER_H