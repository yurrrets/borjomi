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

    return res;
}


CanMessage cmdPing(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_PONG;
    res.devno = 0;
    res.value = NodeConfig.nodeId;

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

    return res;
}

CanMessage cmdAnalogRead(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = analogRead(msg.devno);

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

    return res;
}

CanMessage cmdDigitalRead(const CanMessage &msg)
{
    CanMessage res;
    res.msgno = msg.msgno;
    res.code = CMD_OK;
    res.devno = msg.devno;
    res.value = digitalRead(msg.devno);

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

    return res;
}


