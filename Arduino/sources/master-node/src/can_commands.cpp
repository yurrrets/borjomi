#include "can_commands.h"
#include "config.h"
#include "nodes.h"
#include "log.h"

#define CMD_ANSWER_TIMEOUT_MS (1000)
#define CMD_PING_MULTICAST_TIMEOUT_MS (3000)

CanCommands::CanCommands(MCP_CAN &can, uint8_t canIntPin)
    : CAN0(can), canIntPin(canIntPin), lastAddrId(0), status(S_NO_DATA), busy(false), lastRequestTime(0)
{
}

void CanCommands::setup()
{
    pinMode(canIntPin, INPUT); // Configuring pin for /INT input

    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters
    // disabled.
    if (CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_16MHZ) == CAN_OK)
    {
        LOG_DEBUG(F("CAN: MCP2515 Initialized Successfully!"));
    }
    else
    {
        LOG_ERROR(F("CAN: Error Initializing MCP2515..."));
        return;
    }

    // Change to normal mode to allow messages to be transmitted
    if (CAN0.setMode(MCP_NORMAL) == MCP2515_OK)
    {
        LOG_DEBUG(F("CAN: setMode Successful"));
    }
    else
    {
        LOG_ERROR(F("CAN: setMode failed"));
        return;
    }
}

void CanCommands::loop()
{
    status = read();
    // LOG_DEBUG("status: ", status, ", busy: ", busy);

    if (!busy)
    {
        return;
    }

    if (status == S_NO_DATA)
    {
        auto execTimeMs = millis() - lastRequestTime;
        if (getRequest().code == CMD_PING && getLastRequestAddress() == MULTICAST_NODE)
        {
            if (execTimeMs > CMD_PING_MULTICAST_TIMEOUT_MS)
            {
                // all nodes should answer by this time
                busy = false;
            }
        }
        else
        {
            if (execTimeMs > CMD_ANSWER_TIMEOUT_MS)
            {
                // node didn't answer, timeout error detected
                busy = false;
                status = S_TIMEOUT_ERR;
            }
        }
        return;
    }
    if (status == S_OTHER_NODE)
    {
        // ignore the message
        return;
    }

    // two cases, but anyway we clears busy flag
    //
    // status == S_INVALID_MSG || status == S_INVALID_CRC :
    // error detected, just clear busy flag
    // TODO: maybe it's better just to ignore the message, possibly it's some other activity
    // on the bus, and not an answer to our message
    //
    // status == S_OK :
    // everything ok, just clear busy flag

    busy = false;
}

CanCommands::ReadStatus CanCommands::read()
{
    if (digitalRead(canIntPin))
        return S_NO_DATA; // no data over CAN bus

    long unsigned int rxId;
    unsigned char len = 0;

    // Read data: len = data length
    auto readResult = CAN0.readMsgBuf(&rxId, &len, (byte *)&answer);
    if (readResult != CAN_OK)
    {
        return S_NO_DATA;
    }

    if (rxId != MASTER_NODE)
    {
#ifdef DEBUG
        dbgSerial.print(" Message for non-master node caught. Target node ID: ");
        dbgSerial.print(rxId);
        dbgSerial.println();
#endif
        return S_OTHER_NODE;
    }

    if (len != sizeof(CanMessage))
    {
#ifdef DEBUG
        dbgSerial.println("Invalid CAN msg");
#endif
        return S_INVALID_MSG;
    }

    bool crcOK = CheckCrc8(answer);
#ifdef DEBUG
    dbgSerial.print(" Answer Crc: ");
    dbgSerial.print(crcOK ? "ok" : "failed");
    dbgSerial.print(" AnswerNo: ");
    dbgSerial.print(answer.msgno);
    dbgSerial.print(" AnswerCode: ");
    dbgSerial.print(answer.code);
    dbgSerial.print(" AnswerValue: ");
    dbgSerial.print(answer.value);
    dbgSerial.println();
#endif
    return crcOK ? S_OK : S_INVALID_CRC;
}

uint8_t CanCommands::sendRequest(unsigned long addrId, uint8_t command, uint8_t devno,
                                 uint32_t value)
{
    if (busy)
    {
        return static_cast<uint8_t>(-1);
    }

    busy = true;
    lastRequestTime = millis();
    lastAddrId = addrId;

    request.msgno++;
    if (request.msgno == INVALID_CAN_MSG_NO)
    {
        request.msgno++;
    }
    request.code = command;
    request.devno = devno;
    request.value = value;
    UpdateCrc8(request);

    // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data
    // bytes to send
#ifdef DEBUG
    dbgSerial << "Sending msg: no=" << request.msgno << " code=" << request.code
              << " devno=" << request.devno << " value=" << request.value << endl;
#endif
    byte sndStat = CAN0.sendMsgBuf(addrId, 0, 8, (byte *)&request);
    if (sndStat == CAN_OK)
    {
        // LOG_DEBUG(F("Message Sent Successfully!"));
    }
    else
    {
        busy = false;
        LOG_INFO(F("Error Sending Message: "), (int)sndStat);
    }
    return sndStat;
}
