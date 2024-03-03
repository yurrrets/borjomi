#pragma once

#include <cstdint>

class SPIClass;

#include "../../libraries/SPI/SPI.h"

class SPISettings
{
  public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    {
    }
    SPISettings()
    {
    }
};

class SPIClass
{
  public:
    static void begin()
    {
    }
    static void beginTransaction(SPISettings settings)
    {
    }
    static uint8_t transfer(uint8_t data)
    {
        return 0;
    }
    static uint16_t transfer16(uint16_t data)
    {
        return 0;
    }
    static void transfer(void *buf, std::size_t count)
    {
    }
    static void endTransaction(void)
    {
    }
    static void end()
    {
    }
};
