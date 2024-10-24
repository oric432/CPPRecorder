#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <cstdint>

uint32_t generateSSRC();
void createRTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const float *audioData, size_t length, uint8_t *packetBuffer, size_t packetBufferSize);
void decodeRTPPacket(const uint8_t *packetBuffer, size_t packetSize, uint16_t &sequenceNumber, uint32_t &timestamp, uint32_t &ssrc);
void convertFloatToPCM(const float *input, int16_t *output, size_t length);
void mergeAudioData(int16_t *m_audioBuffer, size_t index, const std::vector<int16_t> &pcmBuffer, size_t pcmDataSize, size_t maxBufferSize);

#endif // UTILS_H
