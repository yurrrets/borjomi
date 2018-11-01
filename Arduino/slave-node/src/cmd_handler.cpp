#include "cmd_handler.h"
#include "cmd_codes.h"
#include "nodes.h"
#include "capabilities.h"


extern SlaveNodeID NodeConfig;

CanMessage cmdVersion(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = 0;
    res.value = MAKE_VERSION(0,2);

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
    res.value |= (NodeConfig.waterSwitchCount & 0x0F) << (CB_WATER_SWITCH-1);
    res.value |= (NodeConfig.soilMoistureCount & 0x0F) << (CB_SOIL_MOISTURE-1);

    res.updateCrc();
    return res;
}


CanMessage cmdSetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;

    if (msg.devno < NodeConfig.soilMoistureCount)
    {
        res.code = CMD_OK;
        res.value = CVAL_ACCEPTED;
        digitalWrite(PINS_WATER_SWITCH[msg.devno], msg.value == CVAL_OPENED ? HIGH : LOW);
    }
    else
    {
        res.code = CMD_ERROR;
        res.value = CVAL_ERR_INVALID_DEV_NO;
    }

    res.updateCrc();
    return res;
}

CanMessage cmdGetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;
    if (msg.devno < NodeConfig.soilMoistureCount)
    {
        res.code = CMD_OK;
        res.value = digitalRead(PINS_WATER_SWITCH[msg.devno]);
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


