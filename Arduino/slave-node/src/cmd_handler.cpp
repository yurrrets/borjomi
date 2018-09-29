#include "cmd_handler.h"
#include "cmd_codes.h"

uint8_t waterSwitchState = 0;

CanMessage cmdSetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = PIN_WATER_SWITCH;
    res.value = CVAL_ACCEPTED;

    waterSwitchState = msg.value;
    digitalWrite(PIN_WATER_SWITCH, waterSwitchState == CVAL_OPENED ? HIGH : LOW);

    res.updateCrc();
    return res;
}

CanMessage cmdGetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = PIN_WATER_SWITCH;
    res.value = waterSwitchState;

    res.updateCrc();
    return res;
}
