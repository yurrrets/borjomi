// CAN Send Example
//

#include <SoftwareSerial.h>

#include "config.h"
#include "stream_ext.h"
#include "cmd_codes.h"
#include "can_message.h"

#include "button.h"
#include "bt_commands.h"
#include "can_commands.h"
#include "nodes.h"
#include "cmd_handler.h"
#include "pc_start.h"

#include <EEPROM.h>

#define BT_RX_PIN   (4)
#define BT_TX_PIN   (10)
#define CAN_CS_PIN  (9)
#define CMD_ANSWER_TIMEOUT (1000)

#define PC_START_PIN (8)   // digital pin

auto &ioSerial = Serial;
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);

StreamExt ioExtSerial(ioSerial);
BTCommandParser ioCommandParser(ioExtSerial);
CanCommands canCommands(CAN_CS_PIN);
MasterNodeID NodeConfig;
PcStart pcStart(PC_START_PIN);

bool lastCommandAnswered;
unsigned long lastCommandMillis;



void setup()
{
#ifdef DEBUG
    auto &dbgSerial = btSerial;
    dbgSerial.begin(9600);
#endif

    pcStart.setup();

    EEPROM.get(0, NodeConfig);
    if (NodeConfig.checkCrc() && NodeConfig.nodeId)
    {
#ifdef DEBUG
        dbgSerial.print("I'm the Master Node ID 0x");
        dbgSerial.print(NodeConfig.nodeId, HEX);
#endif
    }
    else
    {
#ifdef DEBUG
        dbgSerial.println("I'm the Master Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    lastCommandAnswered = true;

    // set the data rate for the SoftwareSerial port
    ioSerial.begin(9600);
//    ioSerial.println("Hello, World!");

    canCommands.setup();

    pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
    pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
    digitalWrite(PINS_PUMP_SWITCH[0], LOW);
}


void loop()
{
    pcStart.loop();

    if (!lastCommandAnswered && (millis() - lastCommandMillis > CMD_ANSWER_TIMEOUT))
    {
        ioCommandParser.answerError(BTERR_TIMEOUT);
        lastCommandAnswered = true;
    }

    if (lastCommandAnswered && ioSerial.available()) {
        BTCommand btCmd = ioCommandParser.read();
        if (btCmd.errcode)
        {
            // send errCode as answer
            ioCommandParser.answerError(btCmd.errcode);
            lastCommandAnswered = true;
        }
        else
        {
            if (btCmd.address == MASTER_NODE)
            {
                processLocalCommand(btCmd, ioCommandParser);
                return;
            }

            lastCommandAnswered = false;
            lastCommandMillis = millis();

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
                canCommands.sendRequest(btCmd.address, btCmd.cmd, btCmd.devno, btCmd.value);
                break;
            case CMD_PING:
                canCommands.sendRequest(btCmd.address, btCmd.cmd, btCmd.devno, btCmd.value);
                if (btCmd.address == MULTICAST_NODE)
                {
                    processLocalCommand(btCmd, ioCommandParser);
                    lastCommandAnswered = true; // we don't know the exact number of answers
                }
                break;
            default:
                ioCommandParser.answerError(BTERR_CMD_NOT_IMPLEMENTED);
                lastCommandAnswered = true;
            }
        }
    }

    // send can msg error
    //        btCommandIO.answerError(BTERR_INVALID_SLAVE_NODE);
    //        lastCommandAnswered = true;

    switch (canCommands.read()) {
    case CanCommands::S_NO_DATA:
    case CanCommands::S_OTHER_NODE:
        break;
    case CanCommands::S_OK:
    {
        if (!lastCommandAnswered)
        {
//            Serial.print("lastProcessCommand: ");
//            Serial.print(lastProcessCommand);
//            Serial.print(" answer.code: ");
//            Serial.println(canCommands.getAnswer().code);

            if (canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                    canCommands.getRequest().code == CMD_PING &&
                    canCommands.getAnswer().code == CMD_PONG)
            {
                ioCommandParser.answerPong(canCommands.getAnswer().value);
            }
            else if (canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                    canCommands.getAnswer().code == CMD_OK)
            {
                // switch reaction depending on command
                switch (canCommands.getRequest().code) {
                case CMD_VERSION:
                    ioCommandParser.answerVersion(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                    break;
                case CMD_CAPABILITIES:
                    ioCommandParser.answerCapabilities(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                    break;
                case CMD_SET_WATER_SWITCH:
                    ioCommandParser.answerOK();
                    break;
                case CMD_GET_WATER_SWITCH:
                    ioCommandParser.answerWaterState(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                case CMD_READ_SOIL_MOISTURE:
                    ioCommandParser.answerSoilMoisture(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                case CMD_ANALOG_READ:
                    ioCommandParser.answerAnalogRead(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                default:
                    ioCommandParser.answerError(BTERR_UNKNOWN_CMD);
                    break;
                }
            }
            else
                ioCommandParser.answerError(BTERR_INVALID_ANSWER);
            lastCommandAnswered = true;
        }
        else if (canCommands.getRequest().code == CMD_PING &&
                 canCommands.getLastRequestAddress() == MULTICAST_NODE &&
                 canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                 canCommands.getAnswer().code == CMD_PONG)
        {
            ioCommandParser.answerPong(canCommands.getAnswer().value);
        }
        break;
    }
    case CanCommands::S_INVALID_MSG:
    case CanCommands::S_INVALID_CRC:
    default:
    {
        if (!lastCommandAnswered)
        {
            ioCommandParser.answerError(BTERR_INVALID_ANSWER);
            lastCommandAnswered = true;
        }
        break;
    }
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
