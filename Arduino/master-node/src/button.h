#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class Button
{
public:
    Button(int pin);
    void setup();
    void loop();

    uint8_t val();
    bool clicked();
    bool changed();
private:
    int pin;
    uint8_t currVal;
    unsigned long prevTime;
};

#endif // BUTTON_H
