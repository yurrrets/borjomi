#include "can_message.h"
#include "tools.h"

void CanMessage::updateCrc()
{
    crc = Crc8Partial(*this);
}

bool CanMessage::checkCrc() const
{
    return crc == Crc8Partial(*this);
}