# Audio Recorder

## Description

This project is an audio recording application built in C++ using PortAudio for audio I/O. It allows users to record audio and manage audio input based on a push-to-talk (PTT) mechanism. The application can be compiled and run on Debian-based systems.

## Features

- Records audio in real-time.
- Supports multiple channels.
- Push-to-talk functionality to control recording.
- Audio data is stored in a buffer for later processing.

## Prerequisites

- CMake (version 3.10 or higher)
- PortAudio library
- Boost library (version 1.74 or higher)

## Installation

1. **Clone the repository:**

   ```bash
   git clone https://github.com/oric432/CPPRecorder.git
   cd CPPRecorder
   ```

2. **Install dependencies:**

   Make sure you have CMake and the required libraries installed. You can install PortAudio and Boost on Debian using the following commands:

   ```bash
   sudo apt update
   sudo apt install libportaudio2 libportaudio-dev libboost-all-dev cmake
   ```

3. **Build the project:**

   Create a build directory and compile the project:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

Run the compiled application from the build directory:

```bash
./CPPRecorder/build
```

To start recording, ensure your microphone is set up and use the designated key to activate the push-to-talk feature.

Space - start PTT 
Enter - stop PTT

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [PortAudio](http://www.portaudio.com/) - the audio I/O library used.
