#ifndef RTP_SERVER_MANAGER_H
#define RTP_SERVER_MANAGER_H

#include <boost/asio.hpp>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <mutex>

typedef struct
{
    uint16_t sequenceNumber;
    uint32_t timestamp;
    std::chrono::milliseconds joinedTime;
} rtpClientInfo;

class RtpServerManager
{
public:
    static const int BIT_DEPTH = 32;
    static const int SAMPLE_RATE = 8000;
    static const int FRAMES_PER_BUFFER = 256;
    static const int CHANNELS = 1;
    static const int DURATION = 10;

public:
    RtpServerManager(boost::asio::io_context &io_ctx);
    ~RtpServerManager();

    void addClient(uint32_t ssrc, rtpClientInfo &client);
    void removeClient(uint32_t ssrc);
    void manageBuffer(uint32_t ssrc, const uint8_t *data, size_t length);

private:
    void startFlushTimer();
    void flushBufferToDisk();

private:
    std::unordered_map<uint32_t, rtpClientInfo> m_rtpSessions;
    enum
    {
        MAX_BUFFER_SIZE = SAMPLE_RATE * CHANNELS * DURATION * BIT_DEPTH / 16
    };
    int16_t m_audioBuffer[MAX_BUFFER_SIZE]{};
    std::chrono::steady_clock::time_point m_serverCreationTime;
    size_t m_fileCount{};
    std::mutex bufferMutex;
    boost::asio::steady_timer flush_timer_;
    boost::asio::io_context &io_ctx;
};

#endif // RTP_SERVER_MANAGER_H