#ifndef BT_COMMANDS_H
#define BT_COMMANDS_H

#include <Arduino.h>
#include "config.h"

// Error codes

/// no errors
#define BTERR_NO_ERROR              (0)

/// empty command; just ignore it
#define BTERR_NO_CMD                (1)

/// unknown command
#define BTERR_UNKNOWN_CMD           (2)

/// reaction on command is not implemented on master node
#define BTERR_CMD_NOT_IMPLEMENTED   (3)

/// command timeout (slave node did not respond;
/// possibly because it isn't connected or doesn't exist)
#define BTERR_TIMEOUT               (4)

/// got some error from slave node during transferring message
#define BTERR_INVALID_ANSWER        (5)

/// slave node return unexpected msg no or error happen during executing the command
#define BTERR_UNEXPECTED_ANSWER     (6)

struct BTCommand
{
    uint8_t errcode;
    unsigned long address;
    uint8_t cmd; // from cmd_codes.h
//    uint8_t pin;
    uint32_t value;
};

class StreamExt;

class BTCommandParser
{
public:
    BTCommandParser(StreamExt &stream);
    BTCommand read();
    void answerError(uint8_t errcode);
    void answerOK();
    void answerWaterState(unsigned long addrId, uint8_t state);

private:
    StreamExt &stream;
};

#endif // BT_COMMANDS_H
