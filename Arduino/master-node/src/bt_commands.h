#ifndef BT_COMMANDS_H
#define BT_COMMANDS_H

#include <Arduino.h>

// Error codes

/// no errors
#define BTERR_NO_ERROR              (0)
/// unknown command
#define BTERR_UNKNOWN_CMD           (1)
/// command timeout (slave node did not respond)
#define BTERR_TIMEOUT               (2)
/// slave node with given address doesn't exist
#define BTERR_INVALID_SLAVE_NODE    (3)

struct BTCommand
{
    uint8_t errcode;
    unsigned long address;
    uint8_t cmd; // from cmd_codes.h
    uint8_t pin;
    uint32_t value;
};

class BTCommandParser
{
public:
    BTCommandParser(Stream &stream);
    BTCommand read();
    void answerError(uint8_t errcode);
    void answerOK();
    void answerWaterState(uint8_t state);

private:
    Stream &stream;
};

#endif // BT_COMMANDS_H
