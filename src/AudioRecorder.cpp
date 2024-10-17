#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(int sampleRate, int framesPerBuffer, int channels, int duration)
    : recorderData{false, sampleRate, framesPerBuffer, channels, false, std::vector<float>(sampleRate * duration * channels), 0, sampleRate * duration}
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

const std::vector<float> &AudioRecorder::getRecordedData() const
{
    return recorderData.recordedSamples;
}

int AudioRecorder::recordCallback(const void *inputBuffer, void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags, void *userData)
{
    paRecorderData *recorderData = static_cast<paRecorderData *>(userData);
    const float *rptr = static_cast<const float *>(inputBuffer);
    float *wptr = &recorderData->recordedSamples[recorderData->frameIndex * recorderData->channels];
    long framesToCalc = recorderData->maxFrameIndex - recorderData->frameIndex;
    long framesLeft = (framesToCalc < framesPerBuffer) ? framesToCalc : framesPerBuffer;

    bool currentPTTState;
    {
        std::lock_guard<std::mutex> lock(recorderData->mtx);
        currentPTTState = recorderData->pttPressed;
    }

    if (!currentPTTState || inputBuffer == nullptr)
    {
        for (long i = 0; i < framesLeft * recorderData->channels; i++)
        {
            *wptr++ = 0.0f; // Silence
        }
    }
    else
    {
        for (long i = 0; i < framesLeft * recorderData->channels; i++)
        {
            *wptr++ = *rptr++; // Audio
        }
    }

    std::cout << framesLeft << " Bytes have been written to the buffer" << '\n';

    recorderData->frameIndex += framesLeft;

    if (recorderData->frameIndex >= recorderData->maxFrameIndex)
    {
        recorderData->recording = false; // Set the recording flag to false in order to stop the main loop
        return paComplete;
    }
    else
    {
        return paContinue;
    }
}
