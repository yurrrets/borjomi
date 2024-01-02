#ifndef CANMESSAGE_H
#define CANMESSAGE_H

#include <Arduino.h>
#include "cmd_codes.h"
#include "tools.h"

#pragma pack(push, 1)

struct CanMessage
{
    uint8_t crc;
    uint8_t msgno;
    uint8_t code;
    uint8_t devno; ///< pin num or other device ordinal num
    uint32_t value;

    CanMessage()
      : crc(0), msgno(0), code(CMD_INVALID), devno(0), value(0)
    { }

    void updateCrc() { crc = Crc8Partial(*this); }
    bool checkCrc() const { return crc == Crc8Partial(*this); }
};

#pragma pack(pop)



#endif // CANMESSAGE_H
