#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <mcp_can.h>
#include "can_message.h"

class CanCommands
{
public:
    enum ReadStatus
    {
        S_NO_DATA = 0,
        S_OK = 1,
        S_OTHER_NODE = 2,  // the message is not for master-node
        S_INVALID_MSG = 3, // length of msg is invalid
        S_INVALID_CRC = 4, // unexpected crc
        S_TIMEOUT_ERR = 5, // target node didn't answer
    };

    /// This is CAN's CS pin
    CanCommands(MCP_CAN &can, uint8_t canIntPin);
    void setup();
    void loop();
    ReadStatus readStatus() const { return status; }
    bool isBusy() const { return busy; }
    unsigned long getLastRequestAddress() const { return lastAddrId; }
    const CanMessage &getRequest() const { return request; }
    const CanMessage &getAnswer() const { return answer; }
    // Sending requests while isBusy is true is forbidden. Function returns uint8_t(-1)
    uint8_t sendRequest(unsigned long addrId, uint8_t command, uint8_t devno, uint32_t value);

private:
    MCP_CAN &CAN0;
    uint8_t canIntPin;
    unsigned long lastAddrId;
    CanMessage request;
    CanMessage answer;
    ReadStatus status;
    bool busy;
    unsigned long lastRequestTime;

    ReadStatus read();
};

#endif // CAN_COMMANDS_H
