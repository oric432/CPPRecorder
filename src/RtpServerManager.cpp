#include "RtpServerManager.h"
#include "AudioFileWriter.h"
#include "Utils.h"
#include <cstring>
#include <thread>

RtpServerManager::RtpServerManager(boost::asio::io_context &io_ctx) : m_serverCreationTime(std::chrono::steady_clock::now()),
                                                                      m_io_ctx(io_ctx), m_flushTimer(io_ctx, std::chrono::seconds(DURATION))
{
    startFlushTimer();
}

void RtpServerManager::addClient(uint32_t ssrc, rtpClientInfo &client)
{
    auto [it, inserted] = m_rtpSessions.emplace(ssrc, client);
    if (!inserted)
    {
        it->second.sequenceNumber = client.sequenceNumber;
        it->second.timestamp = client.timestamp;
    }
    else
    {
        it->second.joinedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_serverCreationTime);
    }
}

void RtpServerManager::removeClient(uint32_t ssrc)
{
    m_rtpSessions.erase(ssrc);
}

void RtpServerManager::manageBuffer(uint32_t ssrc, const uint8_t *data, size_t length)
{
    auto it = m_rtpSessions.find(ssrc);
    if (it == m_rtpSessions.end())
    {
        std::cout << "Client not found" << "\n";
        return;
    }
    rtpClientInfo client = it->second;

    // Ensure length is in float samples, not bytes
    size_t floatSamplesLength = length / sizeof(float);

    // Calculate the index based on joined time and timestamp
    size_t index = ((client.joinedTime.count() * SAMPLE_RATE / 1000) + client.timestamp) % (MAX_BUFFER_SIZE / sizeof(int16_t));

    std::cout << "Joined Time (seconds): " << client.joinedTime.count() / 1000 << std::endl;
    std::cout << "Calculated Index: " << index << std::endl;

    // Allocate a temporary buffer to hold the converted PCM samples
    std::vector<int16_t> pcmBuffer(floatSamplesLength);

    // Convert float audio samples to 16-bit PCM
    convertFloatToPCM(reinterpret_cast<const float *>(data), pcmBuffer.data(), floatSamplesLength);

    // Calculate the size of the PCM data in bytes
    size_t pcmDataSize = pcmBuffer.size() * sizeof(int16_t);

    // Check if the index fits within the buffer size
    if (index + pcmDataSize <= MAX_BUFFER_SIZE / sizeof(int16_t))
    {
        // Copy the PCM data to the buffer
        // std::memcpy(m_audioBuffer + index, pcmBuffer.data(), pcmDataSize);
        mergeAudioData(m_audioBuffer, index, pcmBuffer, pcmBuffer.size(), MAX_BUFFER_SIZE);
        return;
    }
}

void RtpServerManager::startFlushTimer()
{
    // Flush buffer in DURATION seconds interval
    m_flushTimer.async_wait([this](const boost::system::error_code &ec)
                            {
            if (!ec) {
                m_serverCreationTime = std::chrono::steady_clock::now();
                flushBufferToDisk();
                m_flushTimer.expires_at(m_flushTimer.expiry() + std::chrono::seconds(DURATION));
                startFlushTimer();
            } });
}

void RtpServerManager::flushBufferToDisk()
{
    // Create a copy of the current buffer for saving to file
    int16_t audioBufferCopy[MAX_BUFFER_SIZE];
    std::memcpy(audioBufferCopy, m_audioBuffer, MAX_BUFFER_SIZE);
    std::memset(m_audioBuffer, 0, MAX_BUFFER_SIZE);

    // Start a new thread to save the buffer to a WAV file
    AudioFileWriter fileWriter{};
    std::string filename = "audio_output_" + std::to_string(m_fileCount++) + ".wav";
    fileWriter.saveToWav(filename, audioBufferCopy, MAX_BUFFER_SIZE, 16, SAMPLE_RATE, CHANNELS);
}

RtpServerManager::~RtpServerManager()
{
    // Merge all audio output files upon destruction
    AudioFileWriter fileWriter{};
    fileWriter.mergeWavFiles("audio_otuput.wav", "audio_output_", m_fileCount);
}