#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include "can_message.h"

CanMessage cmdSetWaterSwitch(const CanMessage &msg);
CanMessage cmdGetWaterSwitch(const CanMessage &msg);

#endif // CMD_HANDLER_H
