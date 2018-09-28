// CAN Send Example
//

#include <mcp_can.h>
#include <SoftwareSerial.h>

#include <cmd_codes.h>
#include <can_message.h>

#include "config.h"
#include "button.h"
#include "bt_commands.h"


#define CAN0_INT 2
MCP_CAN CAN0(9);     // Set CS to pin 9

long unsigned int rxId;
unsigned char len = 0;

CanMessage msg;
CanMessage answer;
Button btn1(7);
Button btn2(6);
SoftwareSerial btSerial(4, 10, 0);
DbgStream btDbgSerial(btSerial);
BTCommandParser  btCommandIO(btDbgSerial);

int lastProcessCommand;
bool lastCommandAnswered;
unsigned long lastCommandMillis;
#define CMD_ANSWER_TIMEOUT (1000)



void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
#endif

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

    // Change to normal mode to allow messages to be transmitted
    if (CAN0.setMode(MCP_NORMAL) == MCP2515_OK)
    {
#ifdef DEBUG
        Serial.println("setMode Successful");
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("setMode failed");
#endif
    }

    pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
    btn1.setup();
    btn2.setup();

    // set the data rate for the SoftwareSerial port
    btSerial.begin(9600);
//    btSerial.println("Hello, World!");

    lastProcessCommand = CMD_INVALID;
    lastCommandAnswered = true;
}

void setWaterSwitch(uint8_t state, unsigned long addrId)
{
    ++msg.num;
    msg.code = CMD_SET_WATER_SWITCH;
    msg.value = state;
    msg.updateCrc();

    // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
    byte sndStat = CAN0.sendMsgBuf(addrId, 0, 8, (byte *)&msg);
    if(sndStat == CAN_OK)
    {
#ifdef DEBUG
        Serial.println("Message Sent Successfully!");
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.print("Error Sending Message: ");
        Serial.println((int)sndStat);
#endif
//        btCommandIO.answerError(BTERR_INVALID_SLAVE_NODE);
//        lastCommandAnswered = true;
    }
}

void loop()
{
//    if (btSerial.available()) {
//        Serial.write(btSerial.read());
//    }
//    if (Serial.available()) {
//        btSerial.write(Serial.read());
//    }

    if (!lastCommandAnswered && (millis() - lastCommandMillis > CMD_ANSWER_TIMEOUT))
    {
        btCommandIO.answerError(BTERR_INVALID_SLAVE_NODE);
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
            lastProcessCommand = btCmd.cmd;
            lastCommandAnswered = false;
            lastCommandMillis = millis();

            // process bt command
            switch (btCmd.cmd) {
            case CMD_SET_WATER_SWITCH:
                setWaterSwitch(btCmd.value, btCmd.address);
                break;
            default:
                btCommandIO.answerError(BTERR_CMD_NOT_IMPLEMENTED);
                lastCommandAnswered = true;
            }
        }
    }

    btn1.loop();
    if (btn1.changed())
    {
        setWaterSwitch(btn1.val() == HIGH ? CVAL_CLOSED : CVAL_OPENED, 0x140);
    }

    btn2.loop();
    if (btn2.changed())
    {
        setWaterSwitch(btn2.val() == HIGH ? CVAL_CLOSED : CVAL_OPENED, 0x141);
    }
//    delay(100);   // send data per 100ms

    if(!digitalRead(CAN0_INT))
    {
        CAN0.readMsgBuf(&rxId, &len, (byte *)&answer);      // Read data: len = data length
        if (len != sizeof(CanMessage))
        {
            Serial.println("Invalid CAN msg");
            if (!lastCommandAnswered)
            {
                btCommandIO.answerError(BTERR_TIMEOUT);
                lastCommandAnswered = true;
            }
        }
        else
        {
            bool crcOK = answer.checkCrc();
            Serial.print(" Answer Crc: ");
            Serial.print(crcOK ? "ok" : "failed");
            Serial.print(" AnswerNo: ");
            Serial.print(msg.num);
            Serial.print(" AnswerCode: ");
            Serial.print(msg.code);
            Serial.println();

            if (!lastCommandAnswered)
            {
                if (lastProcessCommand == msg.code)
                {
                    // switch reaction
                    btCommandIO.answerOK();
                }
                else
                    btCommandIO.answerError(BTERR_TIMEOUT);
                lastCommandAnswered = true;
            }
        }
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
