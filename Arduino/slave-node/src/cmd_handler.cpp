#include "cmd_handler.h"
#include "cmd_codes.h"

CanMessage cmdVersion(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = 0;
    res.value = MAKE_VERSION(0,2);

    res.updateCrc();
    return res;
}


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

CanMessage cmdGetWaterSwitch(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = PIN_WATER_SWITCH;
    res.value = digitalRead(PIN_WATER_SWITCH);
    // digitalRead is called on OUTPUT pin
    // and returns valid value on AVR based boards
    res.updateCrc();
    return res;
}

CanMessage cmdPing(const CanMessage &msg, unsigned long thisNodeId)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_PONG;
    res.pin = 0;
    res.value = thisNodeId;

    res.updateCrc();
    return res;
}

CanMessage cmdReadSoilMoisture(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = PIN_SOIL_MOISTURE;
    res.value = analogRead(PIN_SOIL_MOISTURE);

    res.updateCrc();
    return res;
}

CanMessage cmdAnalogWrite(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = msg.pin;
    res.value = msg.value;

    analogWrite(msg.pin, msg.value);

    res.updateCrc();
    return res;
}

CanMessage cmdAnalogRead(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = msg.pin;
    res.value = analogRead(msg.pin);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalWrite(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = msg.pin;
    res.value = msg.value;

    digitalWrite(msg.pin, msg.value);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalRead(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = msg.pin;
    res.value = digitalRead(msg.pin);

    res.updateCrc();
    return res;
}

CanMessage cmdDigitalPinMode(const CanMessage &msg)
{
    CanMessage res;
    res.num = msg.num;
    res.code = CMD_OK;
    res.pin = msg.pin;
    res.value = CVAL_ACCEPTED;

    pinMode(msg.pin, msg.value);

    res.updateCrc();
    return res;
}


