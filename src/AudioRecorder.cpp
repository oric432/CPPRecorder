#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(int sampleRate, int framesPerBuffer, int channels, RtpClient rtpClient)
    : m_recorderData{false, sampleRate, framesPerBuffer, channels, false, std::move(rtpClient)}
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
    if (m_recorderData.recording)
    {
        stopRecording();
    }
    Pa_Terminate();
}

bool AudioRecorder::startRecording()
{
    // Open the default device for a stream and set the appropriate data as well as a callback funciton for each sample
    PaError err = Pa_OpenDefaultStream(&m_stream, m_recorderData.channels, 0, paFloat32, m_recorderData.sampleRate, m_recorderData.framesPerBuffer, recordCallback, &m_recorderData);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    // Start streaming
    err = Pa_StartStream(m_stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    m_recorderData.recording = true;
    std::cout << "Audio recording started." << std::endl;
    return true;
}

void AudioRecorder::stopRecording()
{
    m_recorderData.recording = false; // Set the global flag to false before stopping
    Pa_StopStream(m_stream);
    Pa_CloseStream(m_stream);
    std::cout << "Audio recording stopped." << std::endl;
}

bool AudioRecorder::isRecording() const
{
    return m_recorderData.recording;
}

void AudioRecorder::setPTTState(bool state)
{
    std::lock_guard<std::mutex> lock(m_recorderData.mtx);
    m_recorderData.pttPressed = state;
}

int AudioRecorder::recordCallback(const void *inputBuffer, void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags, void *userData)
{
    paRecorderData *recorderData = static_cast<paRecorderData *>(userData);
    const float *rptr = static_cast<const float *>(inputBuffer);
    long framesLeft = framesPerBuffer;

    // Check if PTT is pressed or not
    bool currentPTTState;
    {
        std::lock_guard<std::mutex> lock(recorderData->mtx);
        currentPTTState = recorderData->pttPressed;
    }

    // Allocate space for the audio data
    float audioData[framesLeft * recorderData->channels];

    // If PTT is pressed and there are samples in the buffer
    if (currentPTTState && inputBuffer != nullptr)
    {
        // Copy smaples to audio datas
        std::memcpy(audioData, rptr, framesLeft * sizeof(float) * recorderData->channels);
        std::cout << framesLeft << " Bytes have been written to the buffer" << '\n';

        // Send audio data over RTP / UDP
        recorderData->m_client.sendAudioData(audioData, framesLeft * recorderData->channels);
    }
    else
    {
        // Update the timestamp even though packets were not sent (used for synchronization)
        recorderData->m_client.setTimestamp(recorderData->m_client.getTimestamp() + framesLeft * recorderData->channels);
    }

    return paContinue;
}
