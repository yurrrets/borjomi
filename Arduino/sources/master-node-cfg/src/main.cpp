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

    // CurrentRange = -5..5A
    // NumLevels = 10 bit = 1024
    nodeConfig.dcCurrentCoeffs.A = 10.0 / 1024;
    nodeConfig.dcCurrentCoeffs.B = -5.0;

    // VCC_5V = 4.71
    // VDD_12V = 12.27
    // V_divider = 4.16
    // NumLevels = 10 bit = 1024
    // A = ( VDD_12V / V_divider ) * ( VCC_5V / NumLevels ) = 0.013566636305588941
    nodeConfig.dcVoltageCoeffs.A = ( 12.27 / 4.16 ) * ( 4.71 / 1024 );
    nodeConfig.dcVoltageCoeffs.B = 0;

    nodeConfig.pressureCoeffs.A = 1;
    nodeConfig.pressureCoeffs.B = 0;

    UpdateCrc8(nodeConfig);

    EEPROM.put(0, nodeConfig);
}

void loop()
{

}
