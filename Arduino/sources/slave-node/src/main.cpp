//#include  "/usr/share/arduino/Arduino.mk"

#include <mcp_can.h>
#include <SPI.h>
#include <crc.h>
#include <EEPROM.h>

#include "config.h"
#include "board.h"

#include "can_message.h"
#include "cmd_codes.h"
#include "nodes.h"
#include "capabilities.h"
#include "tools.h"
#include "stream_ext.h"
#include "log.h"

#include "cmd_handler.h"

long unsigned int rxId;
unsigned char len = 0;
//unsigned char rxBuf[8];
//char msg[64];

enum SerialCommandType
{
    CT_UNKNOWN,
    CT_SET,
    CT_GET
};

MCP_CAN CAN0(PIN_CAN0_CS);                               // Set CS to pin 10

CanMessage msg;
SlaveNodeID NodeConfig;

#ifdef DEBUG
Stream &dbgSerial = Serial;
#endif
Print &dbgOutput = Serial;
StreamExt cmdSerial(Serial);

CanMessage processMsg(const CanMessage &msg);
SerialCommandType fillGetSetCmd(StreamExt &stream, uint8_t cmdSetCode, uint8_t cmdGetCode, CanMessage &res);

void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
#endif



    EEPROM.get(0, NodeConfig);
    if (CheckCrc8(NodeConfig) && NodeConfig.nodeId) // nodeId != 0
    {
#ifdef DEBUG
        dbgSerial
                << "I'm the Slave Node ID " << NodeConfig.nodeId
                << ", water switches = " << NodeConfig.waterSwitchCount
                << ", soil moistures = " << NodeConfig.soilMoistureCount
                << endl;
#endif
    }
    else
    {
#ifdef DEBUG
        dbgSerial.println("I'm the Slave Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_16MHZ) == CAN_OK)
    {
#ifdef DEBUG
        dbgSerial.println("MCP2515 Initialized Successfully!");
#endif
    }
    else
    {
#ifdef DEBUG
        dbgSerial.println("Error Initializing MCP2515...");
#endif
    }

    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

    pinMode(PIN_CAN0_INT, INPUT);                            // Configuring pin for /INT input
    
    board_setup();

    cmdSerial.setTimeout(10000);
    cmdSerial.setEchoMode(true);
}

void loop()
{
    if(!digitalRead(PIN_CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
    {
        CAN0.readMsgBuf(&rxId, &len, (byte *)&msg);      // Read data: len = data length
        if (len != sizeof(CanMessage))
        {
            dbgSerial.println("Invalid CAN msg");
            return;
        }

        if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
        {
#ifdef DEBUG
//            dbgSerial.print("Extended ID: 0x");
#endif
            rxId = (rxId & 0x1FFFFFFF);
        }
        else
        {
#ifdef DEBUG
//            dbgSerial.print("Standard ID: 0x");
#endif
        }
#ifdef DEBUG
        dbgSerial.print(rxId, 16);

//        dbgSerial.print(" DLC: ");
        dbgSerial.print(len);
#endif

        if((rxId & 0x40000000) == 0x40000000)    // Determine if message is a remote request frame.
        {
#ifdef DEBUG
//            dbgSerial.println(" REMOTE REQUEST FRAME");
#endif
        }
        else
        {
#ifdef DEBUG
            bool crcOK = CheckCrc8(msg);
            dbgSerial.print(" Crc: ");
            dbgSerial.print(crcOK ? "ok" : "failed");
            dbgSerial.print(" MsgNo: ");
            dbgSerial.print(msg.msgno);
            dbgSerial.print(" MsgCode: ");
            dbgSerial.print(msg.code);
            dbgSerial.println();
#endif

            if ((rxId != NodeConfig.nodeId && msg.code != CMD_PING) ||
                    (msg.code == CMD_PING && rxId != MULTICAST_NODE && rxId != NodeConfig.nodeId))
            {
#ifdef DEBUG
//                dbgSerial.println(" Alien module id, ignoring cmd");
#endif
                return;
            }

            CanMessage answer = processMsg(msg);

            // sending answer
            UpdateCrc8(answer);
            byte sndStat = CAN0.sendMsgBuf(MASTER_NODE, 0, 8, (byte *)&answer);
            if(sndStat == CAN_OK)
            {
        #ifdef DEBUG
//                dbgSerial.println("Message Sent Successfully!");
        #endif
            }
            else
            {
        #ifdef DEBUG
                dbgSerial.print("Error Sending Answer: ");
                dbgSerial.println((int)sndStat);
        #endif
            }
        }

#ifdef DEBUG
//        dbgSerial.println();
#endif
    }

    // cmdSerial input processing
    do
    {
        if (!cmdSerial.available())
        {
            // nothing to process
            break;
        }

        auto is_known_command = [](const char *cmd)
        {
            // add other commands as well
            return !strcmp_P(cmd, PSTR("WATER"));
        };

        char buf[32] = {0};
        static const char Terminators[] = { '=', '?', '*', '\n', '\r' };
        char *curr_buf = buf;
        size_t curr_buf_size = sizeof(buf);
        while (true)
        {
            StreamExt::ReadResult rr = cmdSerial.readBytesUntilOneOf(Terminators, sizeof(Terminators), curr_buf, curr_buf_size);
            if (rr.terminator < 0 || rr.terminator == '\n' || rr.terminator == '\r' || is_known_command(buf))
            {
                break;
            }
            // add terminator char to the end as it's not a known command
            curr_buf[rr.sz] = rr.terminator;
            curr_buf += rr.sz + 1;
            curr_buf_size -= rr.sz;
            if (curr_buf_size) curr_buf_size--;
        }

        bool cmdNeedProcess = true;
        SerialCommandType cmdType = CT_UNKNOWN;

        if (!cmdSerial.available() && cmdSerial.lastReaded() == '\r')
        {
            cmdSerial.write('\n');
        }

        if (!strcmp_P(buf, PSTR("?")) || !strcmp_P(buf, PSTR("HELP")))
        {
            cmdSerial.println(F("?                     this help"));
            cmdSerial.println(F("HELP                  this help"));
            cmdSerial.println(F("WATER=<num>,<state>   set water switch <num> to <state> (0 - off, other - on)"));
            cmdSerial.println(F("WATER?<num>           get water switch <num> state"));
            cmdNeedProcess = false;
        }
        else if (!strcmp_P(buf, PSTR("")))
        {
            // ignore empty lines
            cmdNeedProcess = false;
        }
        else if (!strcmp_P(buf, PSTR("WATER")))
        {
            cmdType = fillGetSetCmd(cmdSerial, CMD_SET_WATER_SWITCH, CMD_GET_WATER_SWITCH, msg);
        }
        else
        {
            cmdSerial.print(F("Unknown command: "));
            cmdSerial.println(buf);
            cmdNeedProcess = false;
        }

        // clear the rest of the buf
        if (cmdSerial.lastReaded() != '\n' && cmdSerial.lastReaded() != '\r')
        {
            cmdSerial.readBytesUntilOneOf(Terminators, sizeof(Terminators), buf, sizeof(buf));
        }

        if (cmdNeedProcess)
        {
            if (!cmdSerial.available() && cmdSerial.lastReaded() == '\r')
            {
                cmdSerial.write('\n');
            }

            if (cmdType == CT_UNKNOWN)
            {
                cmdSerial.print(F("Invalid command: "));
                cmdSerial.println(buf);
            }
            else
            {
                CanMessage answer = processMsg(msg);
                if (answer.code == CMD_ERROR)
                {
                    cmdSerial.print(F("Error "));
                    cmdSerial.println(answer.value);
                }
                else
                {
                    if (cmdType == CT_SET)
                    {
                        cmdSerial.println(F("ok"));
                    }
                    else
                    {
                        cmdSerial.println(answer.value);
                    }
                }
            }
        }
    } while (false);
}

CanMessage processMsg(const CanMessage &msg)
{
    switch (msg.code) {
    case CMD_VERSION:
        return cmdVersion(msg);
    case CMD_CAPABILITIES:
        return cmdCapabilities(msg);
    case CMD_SET_WATER_SWITCH:
        return cmdSetWaterSwitch(msg);
    case CMD_GET_WATER_SWITCH:
        return cmdGetWaterSwitch(msg);
    case CMD_PING:
        if (rxId == MULTICAST_NODE) // need to wait some time to avoid collisions
            delay(NodeConfig.nodeId);
        return cmdPing(msg);
    case CMD_READ_SOIL_MOISTURE:
        return cmdReadSoilMoisture(msg);
    case CMD_ANALOG_WRITE:
        return cmdAnalogWrite(msg);
    case CMD_ANALOG_READ:
        return cmdAnalogRead(msg);
    case CMD_DIGITAL_WRITE:
        return cmdDigitalWrite(msg);
    case CMD_DIGITAL_READ:
        return cmdDigitalRead(msg);
    case CMD_DIGITAL_PIN_MODE:
        return cmdDigitalPinMode(msg);
    default:
        return CanMessage{};
    }
}

SerialCommandType fillGetSetCmd(StreamExt &stream, uint8_t cmdSetCode, uint8_t cmdGetCode, CanMessage &res)
{
    res.msgno = 1;
    if (stream.lastReaded() == '?')
    {
        // GET_XXX command
        // so read an address and return
        res.code = cmdGetCode;
        res.devno = stream.parseInt();
        res.value = 0;
        return CT_GET;
    }
    else if (stream.lastReaded() == '=')
    {
        res.code = cmdSetCode;
        res.devno = stream.parseInt();
        if (stream.readNext() != ',')
        {
            return CT_UNKNOWN;
        }
        res.value = stream.parseInt();
        return CT_SET;
    }
    return CT_UNKNOWN;
}