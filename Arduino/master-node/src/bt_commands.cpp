#include "bt_commands.h"
#include "cmd_codes.h"

BTCommandParser::BTCommandParser(Stream &stream)
    : stream(stream)
{
}

BTCommand BTCommandParser::read()
{
    BTCommand res;
    res.errcode = BTERR_NO_ERROR;

    char buf[16];
    size_t len = stream.readBytesUntil('=', buf, 16);
    buf[len] = '\0';
    if (!strcmp("AT+WATER", buf))
    {
        res.address = stream.parseInt();
        len = stream.readBytes(buf, 1);
        if (len < 1)
        {
            res.errcode = BTERR_TIMEOUT;
            return res;
        }
        if (buf[0] == '?')
        {
            res.cmd = CMD_GET_WATER_SWITCH;
            return res;
        }
        if (buf[0] == ',')
        {
            res.value = stream.parseInt();
            res.cmd = CMD_SET_WATER_SWITCH;
            return res;
        }
    }
    res.errcode = BTERR_UNKNOWN_CMD;
    return res;
}

void BTCommandParser::answerError(uint8_t errcode)
{
    stream.print("ERROR ");
    stream.println(errcode);
}

void BTCommandParser::answerOK()
{
    stream.println("OK");
}

void BTCommandParser::answerWaterState(uint8_t state)
{
    stream.print("+WATER ");
    stream.println(state);
}

