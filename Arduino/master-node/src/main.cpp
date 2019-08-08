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

#include <EEPROM.h>

#define BT_RX_PIN   (4)
#define BT_TX_PIN   (10)
#define CAN_CS_PIN  (9)
#define CMD_ANSWER_TIMEOUT (1000)

SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);
StreamExt btExtSerial(btSerial);
BTCommandParser btCommandIO(btExtSerial);
CanCommands canCommands(CAN_CS_PIN);
MasterNodeID NodeConfig;

bool lastCommandAnswered;
unsigned long lastCommandMillis;



void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
#endif

    EEPROM.get(0, NodeConfig);
    if (NodeConfig.checkCrc() && NodeConfig.nodeId)
    {
#ifdef DEBUG
        Serial.print("I'm the Master Node ID 0x");
        Serial.print(NodeConfig.nodeId, HEX);
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("I'm the Master Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    lastCommandAnswered = true;

    // set the data rate for the SoftwareSerial port
    btSerial.begin(9600);
//    btSerial.println("Hello, World!");

    canCommands.setup();

    pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
    pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
    digitalWrite(PINS_PUMP_SWITCH[0], LOW);
}


void loop()
{
    if (!lastCommandAnswered && (millis() - lastCommandMillis > CMD_ANSWER_TIMEOUT))
    {
        btCommandIO.answerError(BTERR_TIMEOUT);
        lastCommandAnswered = true;
    }

    if (lastCommandAnswered && btSerial.available()) {
        BTCommand btCmd = btCommandIO.read();
        if (btCmd.errcode)
        {
            // send errCode as answer
            btCommandIO.answerError(btCmd.errcode);
            lastCommandAnswered = true;
        }
        else
        {
            if (btCmd.address == MASTER_NODE)
            {
                processLocalCommand(btCmd, btCommandIO);
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
                    processLocalCommand(btCmd, btCommandIO);
                    lastCommandAnswered = true; // we don't know the exact number of answers
                }
                break;
            default:
                btCommandIO.answerError(BTERR_CMD_NOT_IMPLEMENTED);
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
                btCommandIO.answerPong(canCommands.getAnswer().value);
            }
            else if (canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                    canCommands.getAnswer().code == CMD_OK)
            {
                // switch reaction depending on command
                switch (canCommands.getRequest().code) {
                case CMD_VERSION:
                    btCommandIO.answerVersion(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                    break;
                case CMD_CAPABILITIES:
                    btCommandIO.answerCapabilities(canCommands.getLastRequestAddress(), canCommands.getAnswer().value);
                    break;
                case CMD_SET_WATER_SWITCH:
                    btCommandIO.answerOK();
                    break;
                case CMD_GET_WATER_SWITCH:
                    btCommandIO.answerWaterState(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                case CMD_READ_SOIL_MOISTURE:
                    btCommandIO.answerSoilMoisture(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                case CMD_ANALOG_READ:
                    btCommandIO.answerAnalogRead(canCommands.getLastRequestAddress(), canCommands.getAnswer().devno, canCommands.getAnswer().value);
                    break;
                default:
                    btCommandIO.answerError(BTERR_UNKNOWN_CMD);
                    break;
                }
            }
            else
                btCommandIO.answerError(BTERR_INVALID_ANSWER);
            lastCommandAnswered = true;
        }
        else if (canCommands.getRequest().code == CMD_PING &&
                 canCommands.getLastRequestAddress() == MULTICAST_NODE &&
                 canCommands.getAnswer().msgno == canCommands.getRequest().msgno &&
                 canCommands.getAnswer().code == CMD_PONG)
        {
            btCommandIO.answerPong(canCommands.getAnswer().value);
        }
        break;
    }
    case CanCommands::S_INVALID_MSG:
    case CanCommands::S_INVALID_CRC:
    default:
    {
        if (!lastCommandAnswered)
        {
            btCommandIO.answerError(BTERR_INVALID_ANSWER);
            lastCommandAnswered = true;
        }
        break;
    }
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
