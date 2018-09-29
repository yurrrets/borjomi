#ifndef NODES_H
#define NODES_H

#include <Arduino.h>
#include "tools.h"

#define MULTICAST_NODE    (0)
#define UNKNOWN_NODE      (1)
#define MASTER_NODE       (8)
// SLAVE_NODE should be (n*10), where n == 1, 2, 3, ...


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

