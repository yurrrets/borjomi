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
#include "pc_start.h"
#include "common.h"
#include "cap_implementation.h"

#include <EEPROM.h>

#define BT_RX_PIN   (4)
#define BT_TX_PIN   (10)
#define CAN_CS_PIN  (9)

#ifdef PC_START
#define PC_START_PIN (8)   // digital pin
#endif

#ifdef DEBUG
Stream &dbgSerial = Serial;
#endif

//SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);
//auto &ioSerial = btSerial;
auto &ioSerial = Serial;

StreamExt ioExtSerial(ioSerial);
CanCommands canCommands(CAN_CS_PIN);
BTCommandParser ioCommandParser(ioExtSerial);
BTCommandProcessor btCommandProcessor(ioCommandParser, canCommands);

MasterNodeID NodeConfig;
#ifdef PC_START
PcStart pcStart(PC_START_PIN);
#endif



void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
#endif

    // set the data rate for the SoftwareSerial port
    ioSerial.begin(9600);

    // wait for serial port to connect. Needed for native USB
    while (!Serial) {
        ;
    }
//    ioSerial.println("Hello, World!");

#ifdef PC_START
    pcStart.setup();
#endif

    EEPROM.get(0, NodeConfig);
    if (NodeConfig.checkCrc() && NodeConfig.nodeId)
    {
#ifdef DEBUG
        dbgSerial << "I'm the Master Node ID " << NodeConfig.nodeId << endl;
#endif
    }
    else
    {
#ifdef DEBUG
        dbgSerial.println("I'm the Master Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    canCommands.setup();
    btCommandProcessor.setup();

    pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
    pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
    digitalWrite(PINS_PUMP_SWITCH[0], LOW);

    analogReference(EXTERNAL);
}


void loop()
{
#ifdef PC_START
    pcStart.loop();
#endif

    // check if voltage is correct
    if (getControlSwitchVal(CS_DC_ADAPTER) && !IsDcVoltageCorrect())
        setControlSwitchVal(CS_DC_ADAPTER, false); // turn it off

    canCommands.loop();
    btCommandProcessor.loop();
}
