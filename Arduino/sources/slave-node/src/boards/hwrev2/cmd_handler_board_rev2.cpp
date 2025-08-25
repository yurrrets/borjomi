#include "cmd_handler.h"
#include "cmd_codes.h"
#include "nodes.h"
#include "capabilities.h"
#include "board.h"
#include <HardwareSerial.h>

extern SlaveNodeID NodeConfig;

CanMessage cmdCapabilities(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = 0;
    res.value = 0;
    res.value |= makeCapability(CB_WATER_SWITCH, 0x10); // can't use 24, as 16 is max for now
    // TODO: increase the number of bits for storing capability values

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
        set_switch_state(msg.devno, msg.value == CVAL_OPENED);

#ifdef DEBUG
        Serial.print("cmdSetWaterSwitch ok: set ");
        Serial.print(msg.value == CVAL_OPENED ? HIGH : LOW);
        Serial.print(" on device no ");
        Serial.println(msg.devno);
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
        res.value = get_switch_state(msg.devno);
#ifdef DEBUG
        Serial.print("cmdGetWaterSwitch");
#endif
    }
    else
    {
        res.code = CMD_ERROR;
        res.value = CVAL_ERR_INVALID_DEV_NO;
    }
    return res;
}

CanMessage cmdReadSoilMoisture(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.devno = msg.devno;
    res.code = CMD_ERROR;
    res.value = CVAL_ERR_INVALID_DEV_NO;

    return res;
}
