//#include  "/usr/share/arduino/Arduino.mk"

#include <crc.h>
#include <EEPROM.h>

#include "config.h"
#include "nodes.h"


SlaveNodeID NodeConfig;


void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    NodeConfig.nodeId = SLAVE_NODE_ID;
    NodeConfig.waterSwitchCount = 1;
    NodeConfig.soilMoistureCount = SOIL_MOISTURE_COUNT;
    UpdateCrc8(NodeConfig);

    EEPROM.put(0, NodeConfig);
}

void loop()
{

}
