#ifndef AUDIO_FILE_WRITER_H
#define AUDIO_FILE_WRITER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

class AudioFileWriter
{
public:
    void saveToWav(const std::string &filename, const int16_t *audioData, size_t dataSize, int bitsPerSample, int sampleRate, int numChannels);

private:
    void writeWavHeader(std::ofstream &outFile, int sampleRate, int numChannels, int bitsPerSample, int dataSize);
};

#endif // AUDIO_FILE_WRITER_H