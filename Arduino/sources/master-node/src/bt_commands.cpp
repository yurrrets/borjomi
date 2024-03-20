#include "bt_commands.h"
#include "cmd_codes.h"
#include "config.h"
#include "stream_ext.h"
#include "capabilities.h"
#include "cmd_handler.h"
#include "nodes.h"
#include "log.h"

class ReadFinisher
{
  private:
    StreamExt &stream;
    bool leaveUnfinished = false;

  public:
    ReadFinisher(StreamExt &stream) : stream(stream)
    {
    }
    ~ReadFinisher()
    {
        if (leaveUnfinished)
        {
            return;
        }
        if (stream.lastReaded() != '\n')
        {
            stream.readUntil('\n');
        }
    }
    void LeaveUnfinished()
    {
        leaveUnfinished = true;
    }
};

BTCommandParser::BTCommandParser(StreamExt &stream)
    : stream(stream)
{
}

StreamExt &BTCommandParser::getStream()
{
    return stream;
}

bool BTCommandParser::available()
{
    return stream.available();
}

BTCommand BTCommandParser::read()
{
//    Serial.println("BTCommandParser::read");

    BTCommand res = {};
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

    if (!strcmp_P(buf, PSTR("AT+VERSION")))
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

    if (!strcmp_P(buf, PSTR("AT+CAPABILITIES")))
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

    if (!strcmp_P(buf, PSTR("AT+PING")))
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

    if (!strcmp_P(buf, PSTR("AT+WATER")))
    {
        fillGetSetCmd(CMD_SET_WATER_SWITCH, CMD_GET_WATER_SWITCH, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+SOIL")))
    {
        fillSensorCmd(CMD_READ_SOIL_MOISTURE, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+PRESSURE")))
    {
        fillSensorCmd(CMD_READ_PRESSURE_SENSOR, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+CURRENT")))
    {
        fillSensorCmd(CMD_READ_CURRENT_SENSOR, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+VOLTAGE")))
    {
        fillSensorCmd(CMD_READ_VOLTAGE_SENSOR, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+DCADAPTER")))
    {
        fillGetSetCmd(CMD_SET_DC_ADAPTER_SWITCH, CMD_GET_DC_ADAPTER_SWITCH, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+PUMP")))
    {
        fillGetSetCmd(CMD_SET_PUMP_SWITCH, CMD_GET_PUMP_SWITCH, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+ANALOG")))
    {
        fillSensorCmd(CMD_ANALOG_READ, res);
        return res;
    }

    if (!strcmp_P(buf, PSTR("AT+MAINSCENARIO")))
    {
        res.address = MASTER_NODE;
        if (stream.lastReaded() == '?')
        {
            res.cmd = CMD_GET_MAIN_SCENARIO;
            return res;
        }
        else if (stream.lastReaded() == '=')
        {
            res.cmd = CMD_SET_MAIN_SCENARIO;
            rdFinisher.LeaveUnfinished();
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

    // LOG_DEBUG(F("unknown cmd: "), buf);
    res.errcode = BTERR_UNKNOWN_CMD;
    return res;
}

void BTCommandParser::answerError(uint8_t errcode)
{
    stream.print(F("ERROR "));
    stream.println(errcode);
}

void BTCommandParser::answerOK()
{
    stream.println(F("OK"));
}

void BTCommandParser::answerVersion(unsigned long addrId, uint32_t val)
{
    stream.print(F("+VERSION="));
    stream.print(addrId);
    stream.print(",");
    stream.println(val);
}

void BTCommandParser::answerCapabilities(unsigned long addrId, uint32_t val)
{
    stream.print(F("+CAPABILITIES="));
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
    stream.print(F("+PONG="));
    stream.println(addrId);
}

void BTCommandParser::answerWaterState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal(F("+WATER="), addrId, devNo, state);
}

void BTCommandParser::answerDCAdapterState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal(F("+DCADAPTER="), addrId, devNo, state);
}

void BTCommandParser::answerPumpState(unsigned long addrId, uint8_t devNo, uint8_t state)
{
    answerGeneralVal(F("+PUMP="), addrId, devNo, state);
}

void BTCommandParser::answerSoilMoisture(unsigned long addrId, uint8_t devNo, uint16_t val)
{
    answerGeneralVal(F("+SOIL="), addrId, devNo, val);
}

void BTCommandParser::answerPressure(unsigned long addrId, uint8_t devNo, float val)
{
    answerGeneralVal(F("+PRESSURE="), addrId, devNo, val);
}

void BTCommandParser::answerDCCurrent(unsigned long addrId, uint8_t devNo, float val)
{
    answerGeneralVal(F("+CURRENT="), addrId, devNo, val);
}

void BTCommandParser::answerDCVoltage(unsigned long addrId, uint8_t devNo, float val)
{
    answerGeneralVal(F("+VOLTAGE="), addrId, devNo, val);
}

void BTCommandParser::answerAnalogRead(unsigned long addrId, uint8_t pinNo, uint16_t val)
{
    answerGeneralVal(F("+ANALOG="), addrId, pinNo, val);
}

void BTCommandParser::answerDigitalRead(unsigned long addrId, uint8_t pinNo, uint8_t state)
{
    answerGeneralVal(F("+DIGITAL="), addrId, pinNo, state);
}

void BTCommandParser::beginAnswerGetMainScenario()
{
    stream.print(F("AT+MAINSCENARIO="));
}

void BTCommandParser::fillSensorCmd(uint8_t cmdCode, BTCommand &res)
{
    if (stream.lastReaded() == '?')
    {
        res.cmd = cmdCode;
        // first read address
        res.address = stream.parseInt();
        // ensure that next char is comma
        if (stream.readNext() != ',')
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return;
        }
        // and read sensor no
        res.devno = stream.parseInt();
    }
    else
    {
        res.errcode = BTERR_UNKNOWN_CMD;
    }
}

void BTCommandParser::fillGetSetCmd(uint8_t cmdSetCode, uint8_t cmdGetCode, BTCommand &res)
{
    if (stream.lastReaded() == '?')
    {
        // GET_XXX command
        // so read an address and return
        res.cmd = cmdGetCode;
        res.address = stream.parseInt();
        if (stream.readNext() != ',')
        {
            res.errcode = BTERR_UNKNOWN_CMD;
            return;
        }
        res.devno = stream.parseInt();
        return;
    }
    else if (stream.lastReaded() == '=')
        res.cmd = cmdSetCode;
    else
    {
        res.errcode = BTERR_UNKNOWN_CMD;
        return;
    }
    // SET_XXX command
    // first read address
    res.address = stream.parseInt();
    // ensure that next char is comma
    if (stream.readNext() != ',')
    {
        res.errcode = BTERR_UNKNOWN_CMD;
        return;
    }
    // then read dev num
    res.devno = stream.parseInt();
    // ensure that next char is comma
    if (stream.readNext() != ',')
    {
        res.errcode = BTERR_UNKNOWN_CMD;
        return;
    }
    // and read value that should be set
    res.value = stream.parseInt();
}

template <typename StringLike, typename T>
void BTCommandParser::answerGeneralVal(StringLike body, unsigned long addrId, uint8_t devNo, T val)
{
    stream.print(body);
    stream.print(addrId);
    stream.print(",");
    stream.print(devNo);
    stream.print(",");
    stream.println(val);
}

BTCommandProcessor::BTCommandProcessor(BTCommandParser &btCommandParser, CanCommands &canCommands)
    : btCommandParser(btCommandParser), canCommands(canCommands), waitCanMsgno(INVALID_CAN_MSG_NO)
{
}

void BTCommandProcessor::setup()
{
}

void BTCommandProcessor::loop()
{
    if (canCommands.readStatus() == CanCommands::S_TIMEOUT_ERR)
    {
        btCommandParser.answerError(BTERR_TIMEOUT);
        return;
    }

    if (!canCommands.isBusy() && btCommandParser.available()) {
        BTCommand btCmd = btCommandParser.read();
        if (btCmd.errcode)
        {
            // send errCode as answer
            btCommandParser.answerError(btCmd.errcode);
        }
        else
        {
            if (btCmd.address == MASTER_NODE)
            {
                processLocalCommand(btCmd, btCommandParser);
                return;
            }

            // process bt command
            switch (btCmd.cmd) {
            case CMD_VERSION:
            case CMD_CAPABILITIES:
            case CMD_SET_WATER_SWITCH:
            case CMD_GET_WATER_SWITCH:
            case CMD_READ_SOIL_MOISTURE:
            case CMD_READ_PRESSURE_SENSOR:
            case CMD_READ_CURRENT_SENSOR:
            case CMD_SET_DC_ADAPTER_SWITCH:
            case CMD_GET_DC_ADAPTER_SWITCH:
            case CMD_READ_VOLTAGE_SENSOR:
            case CMD_SET_PUMP_SWITCH:
            case CMD_GET_PUMP_SWITCH:
            case CMD_ANALOG_WRITE:
            case CMD_ANALOG_READ:
            case CMD_DIGITAL_WRITE:
            case CMD_DIGITAL_READ:
            case CMD_DIGITAL_PIN_MODE:
            case CMD_PING:
            {
                auto sendResult = canCommands.sendRequest(btCmd.address, btCmd.cmd, btCmd.devno, btCmd.value);
                waitCanMsgno = canCommands.getRequest().msgno;
                if (btCmd.cmd == CMD_PING && btCmd.address == MULTICAST_NODE)
                {
                    processLocalCommand(btCmd, btCommandParser);
                }
                else if (sendResult != CAN_OK)
                {
                    LOG_INFO(F("Error sending can request"));
                    btCommandParser.answerError(BTERR_INVALID_STATE);
                }
                break;
            }
            default:
                btCommandParser.answerError(BTERR_CMD_NOT_IMPLEMENTED);
            }
        }
    }

    // send can msg error
    //        btCommandIO.answerError(BTERR_INVALID_SLAVE_NODE);
    //        lastCommandAnswered = true;

    switch (canCommands.readStatus()) {
    case CanCommands::S_NO_DATA:
    case CanCommands::S_OTHER_NODE:
        break;
    case CanCommands::S_OK:
    {
        if (canCommands.getAnswer().msgno != waitCanMsgno)
        {
            // BTCommandProcessor didn't send this request
            break;
        }
        waitCanMsgno = INVALID_CAN_MSG_NO;

//            Serial.print("lastProcessCommand: ");
//            Serial.print(lastProcessCommand);
//            Serial.print(" answer.code: ");
//            Serial.println(canCommands.getAnswer().code);

        if (canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                canCommands.getRequest().code == CMD_PING &&
                canCommands.getAnswer().code == CMD_PONG)
        {
            btCommandParser.answerPong(canCommands.getAnswer().value);
        }
        else if (canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                canCommands.getAnswer().code == CMD_OK)
        {
            // switch reaction depending on command
            switch (canCommands.getRequest().code) {
            case CMD_VERSION:
                btCommandParser.answerVersion(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                break;
            case CMD_CAPABILITIES:
                btCommandParser.answerCapabilities(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                break;
            case CMD_SET_WATER_SWITCH:
                btCommandParser.answerOK();
                break;
            case CMD_GET_WATER_SWITCH:
                btCommandParser.answerWaterState(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                break;
            case CMD_READ_SOIL_MOISTURE:
                btCommandParser.answerSoilMoisture(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                break;
            case CMD_ANALOG_READ:
                btCommandParser.answerAnalogRead(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                break;
            default:
                btCommandParser.answerError(BTERR_UNKNOWN_CMD);
                break;
            }
        }
        else
        {
            btCommandParser.answerError(BTERR_INVALID_ANSWER);
        }

        break;
    }
    case CanCommands::S_INVALID_MSG:
    case CanCommands::S_INVALID_CRC:
    default:
    {
        if (waitCanMsgno == INVALID_CAN_MSG_NO)
        {
            // BTCommandProcessor didn't send this request
            break;
        }
        waitCanMsgno = INVALID_CAN_MSG_NO;
        btCommandParser.answerError(BTERR_INVALID_ANSWER);
        break;
    }
    }
}
