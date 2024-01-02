#include "cmd_handler.h"
#include "cmd_codes.h"
#include "nodes.h"
#include "capabilities.h"
#include <HardwareSerial.h>


extern SlaveNodeID NodeConfig;

CanMessage cmdVersion(const CanMessage &msg)
{
#ifdef DEBUG
    Serial.println("cmdVersion");
#endif
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = 0;
    res.value = MAKE_VERSION(BORJOMI_VERSION_MJ, BORJOMI_VERSION_MN, BORJOMI_VERSION_REV);

    res.updateCrc();
    return res;
}

CanMessage cmdCapabilities(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = 0;
    res.value = 0;
    res.value |= makeCapability(CB_WATER_SWITCH, NodeConfig.waterSwitchCount);
    res.value |= makeCapability(CB_SOIL_MOISTURE, NodeConfig.soilMoistureCount);

    res.updateCrc();
    return res;
}


CanMessage cmdSetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;

    if (msg.devno < NodeConfig.waterSwitchCount)
    {
        res.code = CMD_OK;
        res.value = CVAL_ACCEPTED;
        digitalWrite(PINS_WATER_SWITCH[msg.devno], msg.value == CVAL_OPENED ? HIGH : LOW);
#ifdef DEBUG
        Serial.print("cmdSetWaterSwitch ok: set ");
        Serial.print(msg.value == CVAL_OPENED ? HIGH : LOW);
        Serial.print(" on device no ");
        Serial.println(msg.devno);
        Serial.print(" on pin ");
        Serial.println(PINS_WATER_SWITCH[msg.devno]);
#endif
    }
    else
    {
        res.code = CMD_ERROR;
        res.value = CVAL_ERR_INVALID_DEV_NO;
#ifdef DEBUG
        Serial.print("cmdSetWaterSwitch failed: invalid device no ");
        Serial.println(msg.devno);
#endif
    }

    res.updateCrc();
    return res;
}

CanMessage cmdGetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;
    if (msg.devno < NodeConfig.waterSwitchCount)
    {
        res.code = CMD_OK;
        res.value = digitalRead(PINS_WATER_SWITCH[msg.devno]);
#ifdef DEBUG
        Serial.print("cmdGetWaterSwitch");
#endif
    }
    else
    {
        res.code = CMD_ERROR;
        res.value = CVAL_ERR_INVALID_DEV_NO;
    }
    // digitalRead is called on OUTPUT pin
    // and returns valid value on AVR based boards
    res.updateCrc();
    return res;
}

CanMessage cmdPing(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_PONG;
    res.devno = 0;
    res.value = NodeConfig.nodeId;

    res.updateCrc();
    return res;
}

CanMessage cmdReadSoilMoisture(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;

    if (msg.devno < NodeConfig.soilMoistureCount)
    {
        res.code = CMD_OK;
        res.value = analogRead(PINS_SOIL_MOISTURE[msg.devno]);
        // inverse value 'cause originally bigger value correspond to dry soil, not wet
        res.value = 1024 - res.value;
    }
    else
    {
        res.code = CMD_ERROR;
        res.value = CVAL_ERR_INVALID_DEV_NO;
    }

    res.updateCrc();
    return res;
}

CanMessage cmdAnalogWrite(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = msg.value;

    analogWrite(msg.devno, msg.value);

    res.updateCrc();
    return res;
}

CanMessage cmdAnalogRead(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = analogRead(msg.devno);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalWrite(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = msg.value;

    digitalWrite(msg.devno, msg.value);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalRead(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = digitalRead(msg.devno);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalPinMode(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = CVAL_ACCEPTED;

    pinMode(msg.devno, msg.value);

    res.updateCrc();
    return res;
}


