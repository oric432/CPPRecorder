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

// Append audio data from one file to the output stream
void AudioFileWriter::appendAudioData(std::ifstream &inputFile, std::ofstream &outputFile)
{
    std::vector<char> buffer(BUFFER_SIZE);

    // Read and write audio data in chunks
    while (inputFile.read(buffer.data(), buffer.size()))
    {
        outputFile.write(buffer.data(), inputFile.gcount());
    }

    // Write any remaining data
    if (inputFile.gcount() > 0)
    {
        outputFile.write(buffer.data(), inputFile.gcount());
    }
}

// Update the WAV header with the correct file size
void AudioFileWriter::updateWavHeader(std::ofstream &outFile, std::uint32_t totalDataSize)
{
    int chunkSize = 36 + totalDataSize;

    // Go to the chunk size field and write the correct size
    outFile.seekp(4, std::ios::beg);
    outFile.write(reinterpret_cast<const char *>(&chunkSize), 4);

    // Go to the data size field and write the correct data size
    outFile.seekp(40, std::ios::beg);
    outFile.write(reinterpret_cast<const char *>(&totalDataSize), 4);
}

// Construct filename from prefix and index
std::string AudioFileWriter::constructFilename(const std::string &prefix, size_t index)
{
    return prefix + std::to_string(index) + ".wav"; // Assuming the files are .wav
}

// Merge WAV files using a prefix and a count
void AudioFileWriter::mergeWavFiles(const std::string &outputFilename, const std::string &prefix, size_t count)
{
    if (count == 0)
    {
        std::cerr << "Error: No input files to merge." << std::endl;
        return;
    }

    // Open the output file for writing
    std::ofstream outFile(outputFilename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Error: Could not open output file for writing!" << std::endl;
        return;
    }

    std::uint32_t totalDataSize = 0;

    // Process the first file to write the header
    std::ifstream firstFile(constructFilename(prefix, 0), std::ios::binary);
    if (!firstFile)
    {
        std::cerr << "Error: Could not open " << constructFilename(prefix, 0) << " for reading!" << std::endl;
        return;
    }

    // Read and write the header of the first file
    std::vector<char> header(44);
    firstFile.read(header.data(), header.size());
    outFile.write(header.data(), header.size());

    // Append the audio data of the first file
    firstFile.seekg(44, std::ios::beg); // Skip header
    appendAudioData(firstFile, outFile);
    totalDataSize += firstFile.tellg() - 44;

    // Process the remaining files (append their data only)
    for (size_t i = 1; i < count; ++i)
    {
        std::ifstream inputFile(constructFilename(prefix, i), std::ios::binary);
        if (!inputFile)
        {
            std::cerr << "Error: Could not open " << constructFilename(prefix, i) << " for reading!" << std::endl;
            continue;
        }

        // Skip the header of the current file
        inputFile.seekg(44, std::ios::beg);

        // Append the audio data to the output file
        appendAudioData(inputFile, outFile);

        // Accumulate total data size
        inputFile.seekg(0, std::ios::end);
        totalDataSize += inputFile.tellg() - 44;
    }

    // Update the header of the output file with the correct data size
    updateWavHeader(outFile, totalDataSize);

    outFile.close();
    std::cout << "Merged audio files saved to " << outputFilename << std::endl;
}