#include "Utils.h"
#include "RtpClient.h"
#include <iostream>

uint32_t generateSSRC()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;
    return dist(gen);
}

void createRTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const float *audioData, size_t length, uint8_t *packetBuffer, size_t packetBufferSize)
{
    // Ensure that the buffer is large enough for the RTP header + audio data
    size_t headerSize = RtpClient::RTP_HEADER_SIZE;
    size_t audioSize = length * sizeof(float);

    if (packetBufferSize < headerSize + audioSize)
    {
        throw std::runtime_error("Buffer size is too small for RTP packet");
    }

    // RTP Header (12 bytes)
    // Version = 2, no padding, no extension, no CSRC
    packetBuffer[0] = (2 << 6) | (0 << 5) | (0 << 4) | (0); // version 2, padding 0, extension 0, CSRC count 0
    packetBuffer[1] = (false << 7) | (payloadType & 0x7F);  // Marker (1 bit) and Payload Type (7 bits)

    // Sequence number (16 bits)
    packetBuffer[2] = (sequenceNumber >> 8) & 0xFF;
    packetBuffer[3] = sequenceNumber & 0xFF;

    // Timestamp (32 bits)
    packetBuffer[4] = (timestamp >> 24) & 0xFF;
    packetBuffer[5] = (timestamp >> 16) & 0xFF;
    packetBuffer[6] = (timestamp >> 8) & 0xFF;
    packetBuffer[7] = timestamp & 0xFF;

    // SSRC (32 bits)
    packetBuffer[8] = (ssrc >> 24) & 0xFF;
    packetBuffer[9] = (ssrc >> 16) & 0xFF;
    packetBuffer[10] = (ssrc >> 8) & 0xFF;
    packetBuffer[11] = ssrc & 0xFF;

    // Copy audio data into the packet buffer
    std::memcpy(packetBuffer + headerSize, audioData, audioSize);
}

void decodeRTPPacket(const uint8_t *packetBuffer, size_t packetSize, uint16_t &sequenceNumber, uint32_t &timestamp, uint32_t &ssrc)
{
    // Check if the packet is large enough to contain the RTP header
    if (packetSize < RtpClient::RTP_HEADER_SIZE)
    {
        throw std::runtime_error("Packet size is too small for RTP header");
    }

    // RTP Header (12 bytes)
    // Extract the Version, Padding, Extension, and CSRC count
    uint8_t versionAndCSRC = packetBuffer[0];
    uint8_t paddingAndType = packetBuffer[1];

    // Version (2 bits)
    uint8_t version = (versionAndCSRC >> 6) & 0x03; // Bits 6-7
    if (version != 2)
    {
        throw std::runtime_error("Unsupported RTP version");
    }

    // Padding (1 bit) and Marker (1 bit) and Payload Type (7 bits)
    bool padding = (paddingAndType >> 7) & 0x01;
    uint8_t payloadType = paddingAndType & 0x7F; // Last 7 bits

    // Sequence Number (16 bits)
    sequenceNumber = (packetBuffer[2] << 8) | packetBuffer[3];

    // Timestamp (32 bits)
    timestamp = (packetBuffer[4] << 24) | (packetBuffer[5] << 16) |
                (packetBuffer[6] << 8) | packetBuffer[7];

    // SSRC (32 bits)
    ssrc = (packetBuffer[8] << 24) | (packetBuffer[9] << 16) |
           (packetBuffer[10] << 8) | packetBuffer[11];
}

void convertFloatToPCM(const float *input, int16_t *output, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        // Clamp the float sample between -1.0 and 1.0 and scale to 16-bit
        float sample = std::clamp(input[i], -1.0f, 1.0f);
        output[i] = static_cast<int16_t>(sample * 32767);
    }
}
