#ifndef CANMESSAGE_H
#define CANMESSAGE_H

#include <Arduino.h>
#include "cmd_codes.h"

#define INVALID_CAN_MSG_NO   (0)

#pragma pack(push, 1)

struct CanMessage
{
    uint8_t crc;
    uint8_t msgno;
    uint8_t code;
    uint8_t devno; ///< pin num or other device ordinal num
    uint32_t value;

    CanMessage()
      : crc(0), msgno(INVALID_CAN_MSG_NO), code(CMD_INVALID), devno(0), value(0)
    { }

    void updateCrc();
    bool checkCrc() const;
};

#pragma pack(pop)

#endif // CANMESSAGE_H
