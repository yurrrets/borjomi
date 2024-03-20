#pragma once

#include "Stream.h"

class Logger : public Stream
{
public:
    size_t write(uint8_t c) override;
};

extern Logger mainLogger;
extern Print &dbgOutput;