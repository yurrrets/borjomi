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

#define BT_RX_PIN   (4)
#define BT_TX_PIN   (10)
#define CAN_CS_PIN  (9)
#define CMD_ANSWER_TIMEOUT (1000)

Button btn1(7);
Button btn2(6);
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);
StreamExt btExtSerial(btSerial);
BTCommandParser btCommandIO(btExtSerial);
CanCommands canCommands(CAN_CS_PIN);

bool lastCommandAnswered;
unsigned long lastCommandMillis;



void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
#endif

    lastCommandAnswered = true;

    btn1.setup();
    btn2.setup();

    // set the data rate for the SoftwareSerial port
    btSerial.begin(9600);
//    btSerial.println("Hello, World!");

    canCommands.setup();
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
            lastCommandAnswered = false;
            lastCommandMillis = millis();

            // process bt command
            switch (btCmd.cmd) {
            case CMD_SET_WATER_SWITCH:
            case CMD_GET_WATER_SWITCH:
                canCommands.sendRequest(btCmd.address, btCmd.cmd, btCmd.value);
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

    btn1.loop();
    if (btn1.changed())
    {
        canCommands.sendRequest(10, CMD_SET_WATER_SWITCH,
                                btn1.val() == HIGH ? CVAL_CLOSED : CVAL_OPENED);
    }

    btn2.loop();
    if (btn2.changed())
    {
        canCommands.sendRequest(20, CMD_SET_WATER_SWITCH,
                                btn1.val() == HIGH ? CVAL_CLOSED : CVAL_OPENED);
    }
//    delay(100);   // send data per 100ms

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
            if (canCommands.getAnswer().num == canCommands.getRequest().num &&
                canCommands.getAnswer().code == CMD_OK)
            {
                // switch reaction depending on command
                switch (canCommands.getRequest().code) {
                case CMD_SET_WATER_SWITCH:
                    btCommandIO.answerOK();
                    break;
                case CMD_GET_WATER_SWITCH:
                    btCommandIO.answerWaterState(canCommands.getAnswer().value);
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
