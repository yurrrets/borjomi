#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <mcp_can.h>
#include "can_message.h"

#define CAN0_INT 2

class CanCommands
{
public:
    enum ReadStatus
    {
        S_NO_DATA = 0,
        S_OK = 1,
        S_OTHER_NODE = 2,  // the message is not for master-node
        S_INVALID_MSG = 3, // length of msg is invalid
        S_INVALID_CRC = 4  // unexpected crc
    };

    /// This is CAN's CS pin
    CanCommands(uint8_t pinCS);
    void setup();
    ReadStatus read();
    unsigned long getLastRequestAddress() const { return lastAddrId; }
    const CanMessage &getRequest() const { return request; }
    const CanMessage &getAnswer() const { return answer; }
    uint8_t sendRequest(unsigned long addrId, uint8_t command, uint8_t devno, uint32_t value);

private:
    MCP_CAN CAN0;
    unsigned long lastAddrId;
    CanMessage request;
    CanMessage answer;
};

#endif // CAN_COMMANDS_H
