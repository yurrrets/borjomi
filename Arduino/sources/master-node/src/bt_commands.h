#ifndef BT_COMMANDS_H
#define BT_COMMANDS_H

#include <Arduino.h>
#include "config.h"
#include "can_commands.h"

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

/// invalid device num index
#define BTERR_INVALID_DEV_ID        (7)

/// command execution is not allowed in current conditions (sensor values, switch states, etc.)
#define BTERR_INVALID_STATE         (8)

struct BTCommand
{
    BTCommand() : errcode(0), cmd(0), devno(0), address(0), value(0) { }
    uint8_t errcode;
    uint8_t cmd; // from cmd_codes.h
    uint8_t devno;
    unsigned long address;
    uint32_t value;
};

class StreamExt;

class BTCommandParser
{
public:
    BTCommandParser(StreamExt &stream);
    bool available();
    BTCommand read();
    void answerError(uint8_t errcode);
    void answerOK();
    void answerVersion(unsigned long addrId, uint32_t val);
    void answerCapabilities(unsigned long addrId, uint32_t val);
    void answerPong(unsigned long addrId);
    void answerWaterState(unsigned long addrId, uint8_t devNo, uint8_t state);
    void answerDCAdapterState(unsigned long addrId, uint8_t devNo, uint8_t state);
    void answerPumpState(unsigned long addrId, uint8_t devNo, uint8_t state);
    void answerSoilMoisture(unsigned long addrId, uint8_t devNo, uint16_t val);
    void answerPressure(unsigned long addrId, uint8_t devNo, float val);
    void answerDCCurrent(unsigned long addrId, uint8_t devNo, float val);
    void answerDCVoltage(unsigned long addrId, uint8_t devNo, float val);
    void answerAnalogRead(unsigned long addrId, uint8_t pinNo, uint16_t val);
    void answerDigitalRead(unsigned long addrId, uint8_t pinNo, uint8_t state);

private:
    StreamExt &stream;
    void fillSensorCmd(uint8_t cmdCode, BTCommand &res);
    void fillGetSetCmd(uint8_t cmdSetCode, uint8_t cmdGetCode, BTCommand &res);
    template <typename StringLike, typename T>
    void answerGeneralVal(StringLike body, unsigned long addrId, uint8_t devNo, T val);
};

class BTCommandProcessor
{
public:
    BTCommandProcessor(BTCommandParser &btCommandParser, CanCommands &canCommands);
    void setup();
    void loop();

private:
    BTCommandParser &btCommandParser;
    CanCommands &canCommands;
    uint8_t waitCanMsgno;
};

#endif // BT_COMMANDS_H
