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

#pragma pack(push, 1)

struct CapabilityInfo
{
    uint8_t capability;
    uint8_t count;
};

struct CbWaterSwitchInfo
{
    uint8_t pin; // digital pin
};

struct CbSoilMoistureSensorInfo
{
    uint8_t pin; // analog pin
};

struct CbPressureSensorInfo
{
    uint8_t pin; // analog pin
    float A;
    float B;
};

struct CbCurrentSensorInfo
{
    uint8_t pin; // analog pin
    float A;
    float B;
};

struct CbDcAdapterSwitchInfo
{
    uint8_t pin; // digital pin
};

struct CbVoltageSensorInfo
{
    uint8_t pin; // analog pin
    float A;
    float B;
};

struct CbPumpSwitchInfo
{
    uint8_t pin; // digital pin
};

#pragma pack(pop)

#endif // CAPABILITIES_H
