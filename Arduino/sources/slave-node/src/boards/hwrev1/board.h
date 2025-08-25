#ifndef BOARD_H
#define BOARD_H

#include <inttypes.h>

#define PIN_CAN0_INT 5                              // Set INT to pin 2
#define PIN_CAN0_CS  9

static const uint8_t PINS_WATER_SWITCH[] = { 3 }; // digital pins
static const uint8_t PINS_SOIL_MOISTURE[] = { 0 }; // analog pins

void board_setup();

#endif // BOARD_H
