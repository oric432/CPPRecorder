#ifndef RTP_CLIENT_H
#define RTP_CLIENT_H

#include "Utils.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>

class RtpClient
{
public:
    static const int RTP_HEADER_SIZE = 12;

public:
    RtpClient(boost::asio::io_context &io_ctx, const std::string &host, const std::string &port);
    ~RtpClient();

    RtpClient(RtpClient &&other) noexcept;
    RtpClient &operator=(RtpClient &&other) noexcept;

    void sendAudioData(const float *audioBuffer, size_t length);

private:
    void createRTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const float *audioData, size_t length, uint8_t *packetBuffer, size_t packetBufferSize);

private:
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_endpoint;
    uint16_t m_sequenceNumber;
    uint32_t m_timestamp;
    uint32_t m_ssrc;
};

#endif // RTP_CLIENT_H
