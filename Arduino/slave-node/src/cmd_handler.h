#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include "can_message.h"

CanMessage cmdVersion(const CanMessage &msg);

CanMessage cmdSetWaterSwitch(const CanMessage &msg);
CanMessage cmdGetWaterSwitch(const CanMessage &msg);
CanMessage cmdPing(const CanMessage &msg, unsigned long thisNodeId);
CanMessage cmdReadSoilMoisture(const CanMessage &msg);

CanMessage cmdAnalogWrite(const CanMessage &msg);
CanMessage cmdAnalogRead(const CanMessage &msg);
CanMessage cmdDigitalWrite(const CanMessage &msg);
CanMessage cmdDigitalRead(const CanMessage &msg);
CanMessage cmdDigitalPinMode(const CanMessage &msg);

#endif // CMD_HANDLER_H
