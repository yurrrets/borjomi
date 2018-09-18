#ifndef CANMESSAGE_H
#define CANMESSAGE_H

#include <Arduino.h>
#include "cmd_codes.h"
#include "tools.h"

#pragma pack(push, 1)

struct CanMessage
{
    uint8_t crc;
    uint8_t num;
    uint8_t code;
    uint8_t pin;
    uint32_t value;

    CanMessage()
      : crc(0), num(0), code(CMD_INVALID), pin(0), value(0)
    { }

    void updateCrc() { crc = Crc8Partial(*this); }
    bool checkCrc() const { return crc == Crc8Partial(*this); }
};

#pragma pack(pop)



#endif // CANMESSAGE_H
