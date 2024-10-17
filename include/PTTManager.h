#ifndef PTT_MANAGER_H
#define PTT_MANAGER_H

#include <thread>
#include <atomic>
#include "AudioRecorder.h"

class PTTManager
{
public:
    PTTManager(AudioRecorder &recorder);
    ~PTTManager();
    void startMonitoring();
    void stopMonitoring();

private:
    void monitorKeyPress();
    AudioRecorder &audioRecorder;
    std::atomic<bool> monitoring;
    std::thread pttThread;

    // Functions to set terminal to non-blocking mode
    void setNonBlocking(bool enable);
};

#endif // PTT_MANAGER_H
