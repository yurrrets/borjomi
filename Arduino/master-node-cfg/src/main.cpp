//#include  "/usr/share/arduino/Arduino.mk"

#include <crc.h>
#include <EEPROM.h>

#include "config.h"
#include "nodes.h"


MasterNodeID nodeConfig;


void setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    nodeConfig.nodeId = MASTER_NODE;
    nodeConfig.reserved1 = 0;
    nodeConfig.reserved2 = 0;
    nodeConfig.reserved3 = 0;

    nodeConfig.dcCurrentCoeffs.A = 1;
    nodeConfig.dcCurrentCoeffs.B = 0;

    nodeConfig.dcVoltageCoeffs.A = 1;
    nodeConfig.dcVoltageCoeffs.B = 0;

    nodeConfig.pressureCoeffs.A = 1;
    nodeConfig.pressureCoeffs.B = 0;

    nodeConfig.updateCrc();

    EEPROM.put(0, nodeConfig);
}

void loop()
{

}
