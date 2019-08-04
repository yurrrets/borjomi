#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include <Arduino.h>

#define CB_WATER_SWITCH        (1)
#define CB_SOIL_MOISTURE       (2)
#define CB_PRESSURE_SENSOR     (3)
#define CB_CURRENT_SENSOR      (4)
#define CB_DC_ADAPTER_SWITCH   (5)
#define CB_VOLTAGE_SENSOR      (6)
#define CB_PUMP_SWITCH         (7)

static const uint8_t PINS_WATER_SWITCH[] = { 3 }; // digital pins
static const uint8_t PINS_SOIL_MOISTURE[] = { 0 }; // analog pins

#pragma pack(push, 1)

struct CbPressureSensorInfo
{
    float A;
    float B;
};

struct CbCurrentSensorInfo
{
    float A;
    float B;
};

struct CbVoltageSensorInfo
{
    float A;
    float B;
};

#pragma pack(pop)

#endif // CAPABILITIES_H
