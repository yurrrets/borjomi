#include "Arduino.h"
#include "ArduinoSims.h"
#include <cassert>

unsigned long millis()
{
    return get_sims_context().micros / 1000;
}

unsigned long micros(void)
{
    return get_sims_context().micros;
}

void delay(unsigned long ms)
{
    get_sims_context().micros += ms * 1000;
}

void delayMicroseconds(unsigned int us)
{
    get_sims_context().micros += us;
}

void pinMode(uint8_t pin, uint8_t mode)
{
    get_sims_context().digitalPinModes[pin] = mode;
}

int analogRead(uint8_t pin)
{
    return get_sims_context().analogPins[pin];
}

void analogWrite(uint8_t pin, int val)
{
    get_sims_context().digitalPins[pin] = val;
    notifyArduinoEvent(AnalogPinChangedEvent{.timeMs = millis(),
                                             .pin = pin,
                                             .value = val,
                                             .pinMode = get_sims_context().digitalPinModes[pin]});
}

int digitalRead(uint8_t pin)
{
    return get_sims_context().digitalPins[pin];
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    get_sims_context().digitalPins[pin] = val;
    notifyArduinoEvent(DigitalPinChangedEvent{.timeMs = millis(),
                                              .pin = pin,
                                              .value = val,
                                              .pinMode = get_sims_context().digitalPinModes[pin]});
}
