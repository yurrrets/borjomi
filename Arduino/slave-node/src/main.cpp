//#include  "/usr/share/arduino/Arduino.mk"

#include <mcp_can.h>
#include <SPI.h>
#include <crc.h>
#include <EEPROM.h>

#include "config.h"

#include "can_message.h"
#include "cmd_codes.h"
#include "nodes.h"
#include "capabilities.h"
#include "tools.h"

#include "cmd_handler.h"

long unsigned int rxId;
unsigned char len = 0;
//unsigned char rxBuf[8];
//char msg[64];

#define CAN0_INT 5                              // Set INT to pin 2
MCP_CAN CAN0(9);                               // Set CS to pin 10




CanMessage msg;
SlaveNodeID NodeConfig;


void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
#endif



    EEPROM.get(0, NodeConfig);
    if (NodeConfig.checkCrc() && NodeConfig.nodeId) // nodeId != 0
    {
        NodeConfig.waterSwitchCount = min(NodeConfig.waterSwitchCount, 1);
        NodeConfig.soilMoistureCount = min(NodeConfig.soilMoistureCount, 1);
#ifdef DEBUG
        Serial.print("I'm the Slave Node ID 0x");
        Serial.print(NodeConfig.nodeId, HEX);
        Serial.print(", water switches = ");
        Serial.print(NodeConfig.waterSwitchCount);
        Serial.print(", soil moistures = ");
        Serial.println(NodeConfig.soilMoistureCount);
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("I'm the Slave Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_16MHZ) == CAN_OK)
    {
#ifdef DEBUG
        Serial.println("MCP2515 Initialized Successfully!");
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("Error Initializing MCP2515...");
#endif
    }

    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

    pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
    pinMode(PINS_WATER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_WATER_SWITCH[0], LOW);
}

void loop()
{
    if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
    {
        CAN0.readMsgBuf(&rxId, &len, (byte *)&msg);      // Read data: len = data length
        if (len != sizeof(CanMessage))
        {
            Serial.println("Invalid CAN msg");
            return;
        }

        if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
        {
#ifdef DEBUG
//            Serial.print("Extended ID: 0x");
#endif
            rxId = (rxId & 0x1FFFFFFF);
        }
        else
        {
#ifdef DEBUG
//            Serial.print("Standard ID: 0x");
#endif
        }
#ifdef DEBUG
        Serial.print(rxId, 16);

//        Serial.print(" DLC: ");
        Serial.print(len);
#endif

        if((rxId & 0x40000000) == 0x40000000)    // Determine if message is a remote request frame.
        {
#ifdef DEBUG
//            Serial.println(" REMOTE REQUEST FRAME");
#endif
        }
        else
        {
#ifdef DEBUG
            bool crcOK = msg.checkCrc();
            Serial.print(" Crc: ");
            Serial.print(crcOK ? "ok" : "failed");
            Serial.print(" MsgNo: ");
            Serial.print(msg.msgno);
            Serial.print(" MsgCode: ");
            Serial.print(msg.code);
            Serial.println();
#endif

            if ((rxId != NodeConfig.nodeId && msg.code != CMD_PING) ||
                    (msg.code == CMD_PING && rxId != MULTICAST_NODE && rxId != NodeConfig.nodeId))
            {
#ifdef DEBUG
//                Serial.println(" Alien module id, ignoring cmd");
#endif
                return;
            }

            CanMessage answer;
            switch (msg.code) {
            case CMD_VERSION:
                answer = cmdVersion(msg);
                break;
            case CMD_CAPABILITIES:
                answer = cmdCapabilities(msg);
                break;
            case CMD_SET_WATER_SWITCH:
                answer = cmdSetWaterSwitch(msg);
                break;
            case CMD_GET_WATER_SWITCH:
                answer = cmdGetWaterSwitch(msg);
                break;
            case CMD_PING:
                if (rxId == MULTICAST_NODE) // need to wait some time to avoid collisions
                    delay(NodeConfig.nodeId);
                answer = cmdPing(msg);
                break;
            case CMD_READ_SOIL_MOISTURE:
                answer = cmdReadSoilMoisture(msg);
                break;
            case CMD_ANALOG_WRITE:
                answer = cmdAnalogWrite(msg);
                break;
            case CMD_ANALOG_READ:
                answer = cmdAnalogRead(msg);
                break;
            case CMD_DIGITAL_WRITE:
                answer = cmdDigitalWrite(msg);
                break;
            case CMD_DIGITAL_READ:
                answer = cmdDigitalRead(msg);
                break;
            case CMD_DIGITAL_PIN_MODE:
                answer = cmdDigitalPinMode(msg);
                break;
            default:
                break;
            }

            // sending answer

            byte sndStat = CAN0.sendMsgBuf(MASTER_NODE, 0, 8, (byte *)&answer);
            if(sndStat == CAN_OK)
            {
        #ifdef DEBUG
//                Serial.println("Message Sent Successfully!");
        #endif
            }
            else
            {
        #ifdef DEBUG
                Serial.print("Error Sending Answer: ");
                Serial.println((int)sndStat);
        #endif
            }
        }

#ifdef DEBUG
//        Serial.println();
#endif
    }
}
