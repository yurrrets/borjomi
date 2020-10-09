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

inline uint32_t makeCapability(uint8_t capability, uint8_t count)
{
    return uint32_t(count & 0x0F) << 4*(capability-1);
}

// on slave node
static const uint8_t PINS_WATER_SWITCH[] = { 3 }; // digital pins
static const uint8_t PINS_SOIL_MOISTURE[] = { 0 }; // analog pins

// on master node
static const uint8_t PINS_DC_ADAPTER_SWITCH[] = { 6 }; // digital pins
static const uint8_t PINS_PUMP_SWITCH[] = { 7 }; // digital pins
static const uint8_t PINS_PRESSURE_SENSOR[] = { 4 }; // analog pins
static const uint8_t PINS_CURRENT_SENSOR[] = { 0 }; // analog pins
static const uint8_t PINS_VOLTAGE_SENSOR[] = { 2 }; // analog pins

#endif // CAPABILITIES_H
