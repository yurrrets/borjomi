#include "can_commands.h"
#include "config.h"
#include "nodes.h"


CanCommands::CanCommands(uint8_t pinCS)
    : CAN0(pinCS), lastAddrId(0)
{
}

void CanCommands::setup()
{
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
}

CanCommands::ReadStatus CanCommands::read()
{
    if(digitalRead(CAN0_INT))
        return S_NO_DATA; // no data over CAN bus

    long unsigned int rxId;
    unsigned char len = 0;

    CAN0.readMsgBuf(&rxId, &len, (byte *)&answer);      // Read data: len = data length

    if (rxId != MASTER_NODE)
    {
#ifdef DEBUG
        Serial.print(" Message for non-master node caught. Target node ID: ");
        Serial.print(rxId);
        Serial.println();
#endif
        return S_OTHER_NODE;
    }

    if (len != sizeof(CanMessage))
    {
        Serial.println("Invalid CAN msg");
        return S_INVALID_MSG;
    }

    bool crcOK = answer.checkCrc();
#ifdef DEBUG
    Serial.print(" Answer Crc: ");
    Serial.print(crcOK ? "ok" : "failed");
    Serial.print(" AnswerNo: ");
    Serial.print(answer.msgno);
    Serial.print(" AnswerCode: ");
    Serial.print(answer.code);
    Serial.print(" AnswerValue: ");
    Serial.print(answer.value);
    Serial.println();
#endif
    return crcOK ? S_OK : S_INVALID_CRC;
}

uint8_t CanCommands::sendRequest(unsigned long addrId, uint8_t command, uint8_t devno, uint32_t value)
{
    lastAddrId = addrId;
    ++request.msgno;
    request.code = command;
    request.devno = devno;
    request.value = value;
    request.updateCrc();

    // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
    byte sndStat = CAN0.sendMsgBuf(addrId, 0, 8, (byte *)&request);
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
    }
    return sndStat;
}
