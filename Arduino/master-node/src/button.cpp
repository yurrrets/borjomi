#include "button.h"

#define CHANGED 0x40
#define CLICK 0x80

Button::Button(int pin)
    :pin(pin)
{

}

void Button::setup()
{
    pinMode(pin,INPUT_PULLUP);
    prevTime=millis();
}

void Button::loop()
{
    uint8_t val=digitalRead(pin);

    currVal &=~CHANGED;
    currVal &=~CLICK;

    if (currVal==val)
        prevTime=millis();
    else{
        if (val==HIGH &&
            millis()-prevTime>100){
            // button released
            currVal=val | CHANGED;
        }
        if (val==LOW &&
            millis()-prevTime>10){
            // button pressed
            currVal=val | CHANGED | CLICK;
        }
    }
}

uint8_t Button::val()
{
    return currVal & HIGH;
}

bool Button::clicked()
{
    return currVal & CLICK;
}

bool Button::changed()
{
    return currVal & CHANGED;
}



