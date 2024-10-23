#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include "RtpClient.h"
#include <portaudio.h>
#include <iostream>
#include <vector>
#include <mutex>

typedef struct
{
    bool recording;
    int sampleRate;
    int framesPerBuffer;
    int channels;
    bool pttPressed;
    RtpClient m_client;
    mutable std::mutex mtx;
} paRecorderData;

class AudioRecorder
{
public:
    AudioRecorder(int sampleRate, int framesPerBuffer, int channels, RtpClient rtpClient);
    ~AudioRecorder();

    bool startRecording();
    void stopRecording();
    bool isRecording() const;
    void setPTTState(bool pttPressed);

private:
    static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

    PaStream *m_stream;
    paRecorderData m_recorderData;
};

#endif // AUDIO_RECORDER_H
