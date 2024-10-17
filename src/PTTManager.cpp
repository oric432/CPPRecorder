#include "PTTManager.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

PTTManager::PTTManager(AudioRecorder &recorder) : audioRecorder(recorder), monitoring(false) {}

PTTManager::~PTTManager()
{
    stopMonitoring();
}

void PTTManager::startMonitoring()
{
    monitoring = true;
    setNonBlocking(true);
    pttThread = std::thread(&PTTManager::monitorKeyPress, this);
}

void PTTManager::stopMonitoring()
{
    monitoring = false;
    if (pttThread.joinable())
    {
        pttThread.join();
    }
    setNonBlocking(false);
}

void PTTManager::setNonBlocking(bool enable)
{
    static bool isNonBlocking = false;
    static struct termios oldt;

    if (enable && !isNonBlocking)
    {
        struct termios newt;
        tcgetattr(STDIN_FILENO, &oldt); // save old settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // disable buffering and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // non-blocking mode
        isNonBlocking = true;
    }
    else if (!enable && isNonBlocking)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
        isNonBlocking = false;
    }
}

void PTTManager::monitorKeyPress()
{
    std::cout << "Press SPACE to activate PTT, 'q' to quit." << std::endl;
    while (monitoring)
    {
        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n > 0)
        {
            if (ch == ' ')
            { // Spacebar for PTT
                audioRecorder.setPTTState(true);
                std::cout << "\nPTT ON (Spacebar pressed)" << std::endl;
            }
            else if (ch == 'q' || ch == 'Q')
            { // 'q' to quit
                audioRecorder.setPTTState(false);
                std::cout << "\nExiting PTT Manager." << std::endl;
                monitoring = false;
                break;
            }
            else
            { // Any other key releases PTT
                audioRecorder.setPTTState(false);
                std::cout << "\nPTT OFF" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
