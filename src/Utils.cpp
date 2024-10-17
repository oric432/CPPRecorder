#include "Utils.h"

uint32_t generateSSRC()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;
    return dist(gen);
}
