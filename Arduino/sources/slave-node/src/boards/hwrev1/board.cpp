#include "board.h"
#include "Arduino.h"

void board_setup()
{
    pinMode(PINS_WATER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_WATER_SWITCH[0], LOW);
}
