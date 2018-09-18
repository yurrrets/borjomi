#ifndef NODES_H
#define NODES_H

#include <Arduino.h>
#include "tools.h"

#define MULTICAST_NODE    (0)
#define UNKNOWN_NODE      (0x100)
#define MASTER_NODE       (0x101)


#pragma pack(push, 1)

struct SlaveNodeID
{
    uint8_t crc;
    long unsigned int nodeId;

    void updateCrc() { crc = Crc8Partial(*this); }
    bool checkCrc() const { return crc == Crc8Partial(*this); }
};

#pragma pack(pop)


#endif // NODES_H

