#include "cap_implementation.h"
#include "capabilities.h"
#include "nodes.h"
#include "common.h"


float readSensorValue(SensorType sensorType)
{
    switch (sensorType) {
    case S_PRESSURE:
    {
        int value = analogRead(PINS_PRESSURE_SENSOR[0]);
        return getNodeConfig().pressureCoeffs.apply(value);
    }
    case S_DC_CURRENT:
    {
        int value = analogRead(PINS_CURRENT_SENSOR[0]);
        return getNodeConfig().dcCurrentCoeffs.apply(value);
    }
    case S_DC_VOLTAGE:
    {
        int value = analogRead(PINS_VOLTAGE_SENSOR[0]);
        return getNodeConfig().dcVoltageCoeffs.apply(value);
    }
    default:
        return 0;
    }
}

uint8_t getControlPin(ControlSwitch controlSwitch)
{
    switch (controlSwitch) {
    case CS_DC_ADAPTER: return PINS_DC_ADAPTER_SWITCH[0];
    case CS_PUMP: return PINS_PUMP_SWITCH[0];
    }
    return (uint8_t)(-1);
}

void setControlSwitchVal(ControlSwitch controlSwitch, bool state)
{
    uint8_t pin = getControlPin(controlSwitch);
    uint8_t val = state ? HIGH : LOW;
    digitalWrite(pin, val);
}

bool getControlSwitchVal(ControlSwitch controlSwitch)
{
    uint8_t pin = getControlPin(controlSwitch);
    uint8_t val = digitalRead(pin);
    return val ? true : false;
}
