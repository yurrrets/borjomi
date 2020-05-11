#include "pc_start.h"

static const unsigned long CLOSE_SWITCH_TIME_MS = 8000;
static const unsigned long OPEN_SWITCH_TIME_MS = 9000;

enum PcStartState
{
    S_PREPARE,
    S_INPROCESS,
    S_DONE
};

PcStart::PcStart(int pin)
    : pin(pin), state(S_PREPARE)
{
}

void PcStart::setup()
{
    pinMode(pin, OUTPUT);
    setSwitch(false); // disconnect pin
}

void PcStart::loop()
{
    if (state == S_DONE)
        return;

    if (millis() > OPEN_SWITCH_TIME_MS && state == S_INPROCESS)
    {
        setSwitch(false); // switch open state
        state = S_DONE;
        return;
    }

    if (millis() > CLOSE_SWITCH_TIME_MS && state == S_PREPARE)
    {
        setSwitch(true); // switch open state
        state = S_INPROCESS;
        return;
    }
}

void PcStart::setSwitch(bool closed)
{
//    if (closed)
//    {
//        // connecting pin with ground
//        pinMode(pin, OUTPUT);
//        digitalWrite(pin, LOW);
//    }
//    else
//    {
//        // disconnect pin
//        pinMode(pin, INPUT);
//    }
    digitalWrite(pin, closed ? HIGH : LOW);
}
