cmake_minimum_required(VERSION 3.10)

project(SomeCodeProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the directory where PortAudio is installed
list(APPEND CMAKE_PREFIX_PATH /usr/local)

find_package(Boost 1.74 REQUIRED COMPONENTS system fiber context)

# Find the PortAudio package
find_package(portaudio REQUIRED)

include_directories(include ${Boost_INCLUDE_DIRS})

file(GLOB SRC_FILES "src/*.cpp")

add_executable(SomeCodeProject ${SRC_FILES})

target_link_libraries(SomeCodeProject ${Boost_LIBRARIES} Boost::fiber Boost::context)

# Link PortAudio to your project
target_link_libraries(SomeCodeProject portaudio)