#include "Logger.h"
#include <iostream>

Logger mainLogger;
Print &dbgOutput = mainLogger;

size_t Logger::write(uint8_t c)
{
    // std::cout.put(c);
    return 1;
}