#include "cmd_handler.h"
#include "capabilities.h"
#include "bt_commands.h"
#include "nodes.h"
#include "cmd_codes.h"
#include "cap_implementation.h"
#include "common.h"


#define CHECK_DEV_NO(devNo) \
    if (btCmd.devno != 0) { \
        btCommandIO.answerError(BTERR_INVALID_DEV_ID); \
        return; \
    }

#define CHECK_SENSOR_NO(type, no) CHECK_DEV_NO(no)
#define CHECK_SWITCH_NO(type, no) CHECK_DEV_NO(no)


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

void cmdNotImplemented(const BTCommand & /*btCmd*/, BTCommandParser &btCommandIO)
{
    btCommandIO.answerError(BTERR_CMD_NOT_IMPLEMENTED);
}

void cmdVersion(const BTCommand & /*btCmd*/, BTCommandParser &btCommandIO)
{
    btCommandIO.answerVersion(
                getNodeConfig().nodeId,
                MAKE_VERSION(BORJOMI_VERSION_MJ, BORJOMI_VERSION_MN, BORJOMI_VERSION_REV));
}

void cmdCapabilities(const BTCommand & /*btCmd*/, BTCommandParser &btCommandIO)
{
    uint32_t value = 0;
    value |= makeCapability(CB_PRESSURE_SENSOR, 1);
    value |= makeCapability(CB_CURRENT_SENSOR, 1);
    value |= makeCapability(CB_DC_ADAPTER_SWITCH, 1);
    value |= makeCapability(CB_VOLTAGE_SENSOR, 1);
    value |= makeCapability(CB_PUMP_SWITCH, 1);
    btCommandIO.answerCapabilities(getNodeConfig().nodeId, value);
}

void cmdPing(const BTCommand & /*btCmd*/, BTCommandParser &btCommandIO)
{
    btCommandIO.answerPong(getNodeConfig().nodeId);
}

void cmdReadPressureSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SENSOR_NO(S_PRESSURE, btCmd.devno);
    float val_bars = readSensorValue(S_PRESSURE);
    btCommandIO.answerPressure(getNodeConfig().nodeId, btCmd.devno, val_bars);
}

void cmdReadCurrentSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SENSOR_NO(S_DC_CURRENT, btCmd.devno);
    float val_A = readSensorValue(S_DC_CURRENT);
    btCommandIO.answerDCCurrent(getNodeConfig().nodeId, btCmd.devno, val_A);
}

void cmdReadVoltageSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SENSOR_NO(S_DC_VOLTAGE, btCmd.devno);
    float val_V = readSensorValue(S_DC_VOLTAGE);
    btCommandIO.answerDCVoltage(getNodeConfig().nodeId, btCmd.devno, val_V);
}

void cmdSetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SWITCH_NO(CS_DC_ADAPTER, btCmd.devno);
    if (btCmd.value == CVAL_ON && !IsDcVoltageCorrect())
    {
        btCommandIO.answerError(BTERR_INVALID_STATE);
        return;
    }
    setControlSwitchVal(CS_DC_ADAPTER, btCmd.value == CVAL_ON);
    btCommandIO.answerOK();
}

void cmdGetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SWITCH_NO(CS_DC_ADAPTER, btCmd.devno);
    bool value = getControlSwitchVal(CS_DC_ADAPTER);
    btCommandIO.answerDCAdapterState(getNodeConfig().nodeId, btCmd.devno, value ? CVAL_ON : CVAL_OFF);
}

void cmdSetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SWITCH_NO(CS_PUMP, btCmd.devno);
    setControlSwitchVal(CS_PUMP, btCmd.value == CVAL_ON);
    btCommandIO.answerOK();
}

void cmdGetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    CHECK_SWITCH_NO(CS_PUMP, btCmd.devno);
    bool value = getControlSwitchVal(CS_PUMP);
    btCommandIO.answerPumpState(getNodeConfig().nodeId, btCmd.devno, value ? CVAL_ON : CVAL_OFF);
}

void cmdAnalogWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    analogWrite(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}

void cmdAnalogRead(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    int value = analogRead(btCmd.devno);
    btCommandIO.answerAnalogRead(getNodeConfig().nodeId, btCmd.devno, value);
}

void cmdDigitalWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    digitalWrite(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}

void cmdDigitalRead(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    int value = digitalRead(btCmd.devno);
    btCommandIO.answerAnalogRead(getNodeConfig().nodeId, btCmd.devno, value);
}

void cmdDigitalPinMode(const BTCommand &btCmd, BTCommandParser &btCommandIO)
{
    pinMode(btCmd.devno, btCmd.value);
    btCommandIO.answerOK();
}
