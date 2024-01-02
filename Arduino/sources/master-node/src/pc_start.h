#ifndef PC_START_H
#define PC_START_H

#include "Arduino.h"

class PcStart
{
public:
    PcStart(int pin);
    void setup();
    void loop();

private:
    int pin;
    uint8_t state;
    void setSwitch(bool closed);
};

#endif // PC_START_H
