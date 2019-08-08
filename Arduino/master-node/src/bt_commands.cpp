#include "bt_commands.h"
#include "cmd_codes.h"
#include "config.h"
#include "stream_ext.h"
#include "capabilities.h"


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

    static const char Terminators[] = { '=', '?', '*', '\n' };
    StreamExt::ReadResult rr = stream.readBytesUntilOneOf(Terminators, sizeof(Terminators), buf, sizeof(buf));
    buf[rr.sz] = '\0';

    if (!strcmp("AT+VERSION", buf))
    {
        if (stream.lastReaded() == '?')
        {
            res.cmd = CMD_VERSION;
            res.address = stream.parseInt();
            return res;
        }
        else
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
    }

    if (!strcmp("AT+CAPABILITIES", buf))
    {
        if (stream.lastReaded() == '?')
        {
            res.cmd = CMD_CAPABILITIES;
            res.address = stream.parseInt();
            return res;
        }
        else
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
    }

    if (!strcmp("AT+PING", buf))
    {
        res.cmd = CMD_PING;
        if (stream.lastReaded() == '*')
        {
            res.address = 0;
            return res;
        }
        if (stream.lastReaded() != '?')
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
        res.address = stream.parseInt();
        return res;
    }

    if (!strcmp("AT+WATER", buf))
    {
        if (stream.lastReaded() == '?')
        {
            // GET_WATER_SWITCH command
            // so read an address and return
            res.cmd = CMD_GET_WATER_SWITCH;
            res.address = stream.parseInt();
            if (stream.readNext() != ',')
            {
                res.errcode = BTERR_UNKNOWN_CMD;
                return res;
            }
            res.devno = stream.parseInt();
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
        // then read water switch num
        res.devno = stream.parseInt();
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

    if (!strcmp("AT+SOIL", buf))
    {
        if (stream.lastReaded() == '?')
        {
            res.cmd = CMD_READ_SOIL_MOISTURE;
            // first read address
            res.address = stream.parseInt();
            // ensure that next char is comma
            if (stream.readNext() != ',')
            {
                res.errcode = BTERR_UNKNOWN_CMD;
                return res;
            }
            // and read soil moisture no
            res.devno = stream.parseInt();
            return res;
        }
        else
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
    }

    if (!strcmp("AT+ANALOG", buf))
    {
        if (stream.lastReaded() == '?')
        {
            res.cmd = CMD_ANALOG_READ;
            // first read address
            res.address = stream.parseInt();
            // ensure that next char is comma
            if (stream.readNext() != ',')
            {
                res.errcode = BTERR_UNKNOWN_CMD;
                return res;
            }
            // and read pin no
            res.devno = stream.parseInt();
            return res;
        }
        else
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return res;
        }
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

void BTCommandParser::answerVersion(unsigned long addrId, uint32_t val)
{
    stream.print("+VERSION=");
    stream.print(addrId);
    stream.print(",");
    stream.println(val);
}

void BTCommandParser::answerCapabilities(unsigned long addrId, uint32_t val)
{
    stream.print("+CAPABILITIES=");
    stream.print(addrId);
    stream.print(",");
    uint8_t cap = 1;
    while (val)
    {
        uint8_t cnt = val & 0x0F;
        if (cnt)
        {
            switch (cap) {
            case CB_WATER_SWITCH:
                stream.print("WATER:");
                break;
            case CB_SOIL_MOISTURE:
                stream.print("SOIL:");
                break;
            case CB_PRESSURE_SENSOR:
                stream.print("PRESSURE:");
                break;
            case CB_CURRENT_SENSOR:
                stream.print("CURRENT:");
                break;
            case CB_DC_ADAPTER_SWITCH:
                stream.print("DCADAPTER:");
                break;
            case CB_VOLTAGE_SENSOR:
                stream.print("VOLTAGE:");
                break;
            case CB_PUMP_SWITCH:
                stream.print("PUMP:");
                break;
            default:
                stream.print("???:");
                break;
            }
            stream.print(cnt);
            stream.print(";");
        }
        val = val >> 4;
        ++cap;
    }
    stream.println();
}

void BTCommandParser::answerPong(unsigned long addrId)
{
    stream.print("+PONG=");
    stream.println(addrId);
}

void BTCommandParser::answerWaterState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal("+WATER=", addrId, devNo, state);
}

void BTCommandParser::answerDCAdapterState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal("+DCADAPTER=", addrId, devNo, state);
}

void BTCommandParser::answerPumpState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal("+PUMP=", addrId, devNo, state);
}

void BTCommandParser::answerSoilMoisture(unsigned long addrId, uint8_t devNo, uint16_t val)
{
    answerGeneralVal("+SOIL=", addrId, devNo, val);
}

void BTCommandParser::answerPressure(unsigned long addrId, uint8_t devNo, uint16_t val)
{
    answerGeneralVal("+PRESSURE=", addrId, devNo, val);
}

void BTCommandParser::answerDCCurrent(unsigned long addrId, uint8_t devNo, uint16_t val)
{
    answerGeneralVal("+CURRENT=", addrId, devNo, val);
}

void BTCommandParser::answerDCVoltage(unsigned long addrId, uint8_t devNo, uint16_t val)
{
    answerGeneralVal("+VOLTAGE=", addrId, devNo, val);
}

void BTCommandParser::answerAnalogRead(unsigned long addrId, uint8_t pinNo, uint16_t val)
{
    answerGeneralVal("+ANALOG=", addrId, pinNo, val);
}

void BTCommandParser::answerDigitalRead(unsigned long addrId, uint8_t pinNo, uint8_t state)
{
    answerGeneralVal("+DIGITAL=", addrId, devNo, val);
}

template <typename T>
void BTCommandParser::answerGeneralVal(const char *body, unsigned long addrId, uint8_t devNo, T val)
{
    stream.print(body);
    stream.print(addrId);
    stream.print(",");
    stream.print(devNo);
    stream.print(",");
    stream.println(val);
}
