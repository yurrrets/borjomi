#include "cmd_handler.h"
#include "cmd_codes.h"


CanMessage cmdSetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = PIN_WATER_SWITCH;
    res.value = CVAL_ACCEPTED;

    digitalWrite(PIN_WATER_SWITCH, msg.value == CVAL_OPENED ? HIGH : LOW);

    res.updateCrc();
    return res;
}
