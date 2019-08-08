#include "cmd_handler.h"
#include "capabilities.h"
#include "bt_commands.h"
#include "nodes.h"
#include "cmd_codes.h"


extern MasterNodeID NodeConfig;


void processLocalCommand(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    switch (btCmd.cmd) {
    case CMD_VERSION:
        cmdVersion(btCmd, btCommandIO);
        break;
    case CMD_PING:
        cmdPing(btCmd, btCommandIO);
        break;
    case CMD_CAPABILITIES:
        cmdCapabilities(btCmd, btCommandIO);
        break;
    case CMD_SET_WATER_SWITCH:
    case CMD_GET_WATER_SWITCH:
        cmdNotImplemented(btCmd, btCommandIO);
        break;
    case CMD_READ_PRESSURE_SENSOR:
        cmdReadPressureSensor(btCmd, btCommandIO);
        break;
    case CMD_READ_CURRENT_SENSOR:
        cmdReadCurrentSensor(btCmd, btCommandIO);
        break;
    case CMD_SET_DC_ADAPTER_SWITCH:
        cmdSetDCAdapterSwitch(btCmd, btCommandIO);
        break;
    case CMD_GET_DC_ADAPTER_SWITCH:
        cmdGetDCAdapterSwitch(btCmd, btCommandIO);
        break;
    case CMD_READ_VOLTAGE_SENSOR:
        cmdReadVoltageSensor(btCmd, btCommandIO);
        break;
    case CMD_SET_PUMP_SWITCH:
        cmdSetPumpSwitch(btCmd, btCommandIO);
        break;
    case CMD_GET_PUMP_SWITCH:
        cmdGetPumpSwitch(btCmd, btCommandIO);
        break;
    case CMD_ANALOG_WRITE:
        cmdAnalogWrite(btCmd, btCommandIO);
        break;
    case CMD_ANALOG_READ:
        cmdAnalogRead(btCmd, btCommandIO);
        break;
    case CMD_DIGITAL_WRITE:
        cmdDigitalWrite(btCmd, btCommandIO);
        break;
    case CMD_DIGITAL_READ:
        cmdDigitalRead(btCmd, btCommandIO);
        break;
    case CMD_DIGITAL_PIN_MODE:
        cmdDigitalPinMode(btCmd, btCommandIO);
        break;
    default:
        btCommandIO.answerError(BTERR_CMD_NOT_IMPLEMENTED);
    }
}

void cmdNotImplemented(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    btCommandIO.answerError(BTERR_CMD_NOT_IMPLEMENTED);
}

void cmdVersion(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    btCommandIO.answerVersion(
                NodeConfig.nodeId,
                MAKE_VERSION(BORJOMI_VERSION_MJ, BORJOMI_VERSION_MN, BORJOMI_VERSION_REV));
}

void cmdCapabilities(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    uint32_t value = 0;
    value |= makeCapability(CB_PRESSURE_SENSOR, 1);
    value |= makeCapability(CB_CURRENT_SENSOR, 1);
    value |= makeCapability(CB_DC_ADAPTER_SWITCH, 1);
    value |= makeCapability(CB_VOLTAGE_SENSOR, 1);
    value |= makeCapability(CB_PUMP_SWITCH, 1);
    btCommandIO.answerCapabilities(NodeConfig.nodeId, value);
}

void cmdPing(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    btCommandIO.answerPong(NodeConfig.nodeId);
}

void cmdReadPressureSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    int value = analogRead(PINS_PRESSURE_SENSOR[btCmd.devno]);
    float val_bars = NodeConfig.pressureCoeffs.apply(value);
    btCommandIO.answerPressure(NodeConfig.nodeId, btCmd.devno, val_bars);
}

void cmdReadCurrentSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    int value = analogRead(PINS_CURRENT_SENSOR[btCmd.devno]);
    float val_A = NodeConfig.dcCurrentCoeffs.apply(value);
    btCommandIO.answerDCCurrent(NodeConfig.nodeId, btCmd.devno, val_A);
}

void cmdReadVoltageSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    int value = analogRead(PINS_VOLTAGE_SENSOR[btCmd.devno]);
    float val_V = NodeConfig.dcVoltageCoeffs.apply(value);
    btCommandIO.answerDCVoltage(NodeConfig.nodeId, btCmd.devno, val_V);
}

void cmdSetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    digitalWrite(PINS_DC_ADAPTER_SWITCH[btCmd.devno], btCmd.value == CVAL_ON ? HIGH : LOW);
    btCommandIO.answerOK();
}

void cmdGetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    int value = digitalRead(PINS_DC_ADAPTER_SWITCH[btCmd.devno]);
    btCommandIO.answerDCAdapterState(NodeConfig.nodeId, btCmd.devno, value ? CVAL_ON : CVAL_OFF);
}

void cmdSetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    digitalWrite(PINS_PUMP_SWITCH[btCmd.devno], btCmd.value == CVAL_ON ? HIGH : LOW);
    btCommandIO.answerOK();
}

void cmdGetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    if (btCmd.devno != 0)
    {
        btCommandIO.answerError(BTERR_INVALID_ANSWER);
        return;
    }
    int value = digitalRead(PINS_PUMP_SWITCH[btCmd.devno]);
    btCommandIO.answerPumpState(NodeConfig.nodeId, btCmd.devno, value ? CVAL_ON : CVAL_OFF);
}

void cmdAnalogWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    analogWrite(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}

void cmdAnalogRead(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    int value = analogRead(btCmd.devno);
    btCommandIO.answerAnalogRead(NodeConfig.nodeId, btCmd.devno, value);
}

void cmdDigitalWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    digitalWrite(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}

void cmdDigitalRead(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    int value = digitalRead(btCmd.devno);
    btCommandIO.answerAnalogRead(NodeConfig.nodeId, btCmd.devno, value);
}

void cmdDigitalPinMode(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    pinMode(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}
