#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(int sampleRate, int framesPerBuffer, int channels, RtpClient rtpClient)
    : recorderData{false, sampleRate, framesPerBuffer, channels, false, std::move(rtpClient)}
{
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        throw std::runtime_error("Failed to initialize PortAudio");
    }
}

AudioRecorder::~AudioRecorder()
{
    if (recorderData.recording)
    {
        stopRecording();
    }
    Pa_Terminate();
}

bool AudioRecorder::startRecording()
{
    PaError err = Pa_OpenDefaultStream(&stream, recorderData.channels, 0, paFloat32, recorderData.sampleRate, recorderData.framesPerBuffer, recordCallback, &recorderData);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    recorderData.recording = true;
    std::cout << "Audio recording started." << std::endl;
    return true;
}

void AudioRecorder::stopRecording()
{
    recorderData.recording = false; // Set the global flag to false before stopping
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    std::cout << "Audio recording stopped." << std::endl;
}

bool AudioRecorder::isRecording() const
{
    return recorderData.recording;
}

void AudioRecorder::setPTTState(bool state)
{
    std::lock_guard<std::mutex> lock(recorderData.mtx);
    recorderData.pttPressed = state;
}

int AudioRecorder::recordCallback(const void *inputBuffer, void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags, void *userData)
{
    paRecorderData *recorderData = static_cast<paRecorderData *>(userData);
    const float *rptr = static_cast<const float *>(inputBuffer);
    long framesLeft = framesPerBuffer;

    bool currentPTTState;
    {
        std::lock_guard<std::mutex> lock(recorderData->mtx);
        currentPTTState = recorderData->pttPressed;
    }

    float audioData[framesLeft * recorderData->channels];

    if (!currentPTTState || inputBuffer == nullptr)
    {
        std::fill(audioData, audioData + framesLeft * recorderData->channels, 0.0f);
    }
    else
    {
        std::memcpy(audioData, rptr, framesLeft * sizeof(float) * recorderData->channels);
    }

    std::cout << framesLeft << " Bytes have been written to the buffer" << '\n';

    recorderData->m_client.sendAudioData(audioData, framesLeft * recorderData->channels);

    return paContinue;
}
