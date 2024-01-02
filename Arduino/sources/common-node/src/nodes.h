#ifndef NODES_H
#define NODES_H

#include <Arduino.h>
#include "tools.h"
#include "capabilities.h"


#define MULTICAST_NODE    (0)
#define UNKNOWN_NODE      (1)
#define MASTER_NODE       (8)
// SLAVE_NODE should be (n*10), where n == 1, 2, 3, ...


#pragma pack(push, 1)

struct CbLinearModelInfo
{
    float A;
    float B;

    template <typename T>
    float apply(T x) { return A*x + B; }
};

struct MasterNodeID
{
    uint8_t crc;
    long unsigned int nodeId;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    CbLinearModelInfo pressureCoeffs;
    CbLinearModelInfo dcCurrentCoeffs;
    CbLinearModelInfo dcVoltageCoeffs;

    void updateCrc() { crc = Crc8Partial(*this); }
    bool checkCrc() const { return crc == Crc8Partial(*this); }
};

struct SlaveNodeID
{
    uint8_t crc;
    long unsigned int nodeId;
    uint8_t waterSwitchCount;
    uint8_t soilMoistureCount;

    void updateCrc() { crc = Crc8Partial(*this); }
    bool checkCrc() const { return crc == Crc8Partial(*this); }
};

#pragma pack(pop)


#endif // NODES_H

