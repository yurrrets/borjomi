#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

class BTCommand;
class BTCommandParser;

void processLocalCommand(const BTCommand &btCmd, BTCommandParser &btCommandIO);

void cmdNotImplemented(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdVersion(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdCapabilities(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdPing(const BTCommand &btCmd, BTCommandParser &btCommandIO);

void cmdReadPressureSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdReadCurrentSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdReadVoltageSensor(const BTCommand &btCmd, BTCommandParser &btCommandIO);

void cmdSetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdGetDCAdapterSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdSetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdGetPumpSwitch(const BTCommand &btCmd, BTCommandParser &btCommandIO);

void cmdAnalogRead(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdAnalogWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdAnalogRead(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdDigitalWrite(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdDigitalRead(const BTCommand &btCmd, BTCommandParser &btCommandIO);
void cmdDigitalPinMode(const BTCommand &btCmd, BTCommandParser &btCommandIO);

#endif // CMD_HANDLER_H
