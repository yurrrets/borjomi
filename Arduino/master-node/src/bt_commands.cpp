#include "bt_commands.h"
#include "cmd_codes.h"
#include "config.h"


class ReadFinisher
{
private:
    Stream &stream;
public:
    ReadFinisher(Stream &stream) : stream(stream) { }
    ~ReadFinisher() { while (stream.available()) stream.read(); }
};

BTCommandParser::BTCommandParser(Stream &stream)
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
    char buf[16];

//    int c = stream.read();
//    if (c < 0)
//    {
//        res.errcode = BTERR_NO_CMD;
//        return res;
//    }

//    Serial.print("num: ");
//    Serial.println(c);

//    buf[0] = (char)c;
    size_t len = stream.readBytesUntil('=', buf, 16);
//    len++;
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

#ifdef DEBUG
    Serial.print("BT Received cmd: ");
    Serial.println(buf);
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




DbgStream::DbgStream(Stream &wrk)
    : wrk(wrk)
{
}

size_t DbgStream::write(uint8_t val)
{
#ifdef DEBUG
    Serial.write(val);
#endif
    return wrk.write(val);
}

int DbgStream::available()
{
    return wrk.available();
}

int DbgStream::read()
{
    int res = wrk.read();
#ifdef DEBUG
    if (res >= 0)
        Serial.write(res);
#endif
    return res;
}

int DbgStream::peek()
{
    return wrk.peek();
}

void DbgStream::flush()
{
    wrk.flush();
}
