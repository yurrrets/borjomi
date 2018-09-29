#include "bt_commands.h"
#include "cmd_codes.h"
#include "config.h"
#include "stream_ext.h"

class ReadFinisher
{
private:
    StreamExt &stream;
public:
    ReadFinisher(StreamExt &stream) : stream(stream) { }
    ~ReadFinisher() {
//        Serial.print("_last readed (in hex): ");
//        Serial.print(stream.lastReaded(), 16);
        if (stream.lastReaded() != '\n')
        {
            /*size_t len = */ stream.readUntil('\n');
//            Serial.print(" readed next len: ");
//            Serial.print(len);
        }
//        Serial.println();
//        while (stream.available())
//            stream.read();
    }
};

BTCommandParser::BTCommandParser(StreamExt &stream)
    : stream(stream)
{
}

BTCommand BTCommandParser::read()
{
//    Serial.println("BTCommandParser::read");

    BTCommand res;
    if (!stream.available())
    {
        res.errcode = BTERR_NO_CMD;
        return res;
    }

    ReadFinisher rdFinisher(stream);

    res.errcode = BTERR_NO_ERROR;
    char buf[32];

    static const char Terminators[] = { '=', '?' };
    StreamExt::ReadResult rr = stream.readBytesUntilOneOf(Terminators, sizeof(Terminators), buf, sizeof(buf));
    buf[rr.sz] = '\0';
    if (!strcmp("AT+WATER", buf))
    {
        if (stream.lastReaded() == '?')
        {
            // GET_WATER_SWITCH command
            // so read an address and return
            res.cmd = CMD_GET_WATER_SWITCH;
            res.address = stream.parseInt();
            return res;
        }
        else if (stream.lastReaded() == '=')
            res.cmd = CMD_SET_WATER_SWITCH;
        else
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
        // SET_WATER_SWITCH command
        // first read address
        res.address = stream.parseInt();
        // ensure that next char is comma
        if (stream.readNext() != ',')
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
        // and read value that should be set
        res.value = stream.parseInt();
        return res;
    }

#ifdef DEBUG
    Serial.print("BT Received cmd: ");
    Serial.print(buf);
    Serial.print(" len: ");
    Serial.print(rr.sz);
    Serial.print(" hex bytes: ");
    for (uint8_t i = 0; i < rr.sz; i++)
    {
        Serial.print((int)buf[i], 16);
        Serial.print(" ");
    }
    Serial.println();
#endif
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




