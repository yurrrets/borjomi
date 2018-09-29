//#include  "/usr/share/arduino/Arduino.mk"

#include <crc.h>
#include <EEPROM.h>

#include "config.h"
#include "nodes.h"


SlaveNodeID nodeID;


void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    nodeID.nodeId = SLAVE_NODE_ID;
    nodeID.updateCrc();

    EEPROM.put(0, nodeID);
}

void loop()
{

}
