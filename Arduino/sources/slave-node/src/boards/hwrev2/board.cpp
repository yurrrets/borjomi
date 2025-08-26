#include "board.h"
#include "Arduino.h"
#include "SPI.h"

static uint32_t switches_state = 0;

void board_setup()
{
    pinMode(PIN_SHIFT_REG_LATCH, OUTPUT);
    pinMode(PIN_SHIFT_REG_DATA, OUTPUT);
    pinMode(PIN_SHIFT_REG_CLOCK, OUTPUT);
    // update initial values
    switches_state = 0;
    set_switch_state(0, 0);
}

void set_switch_state(int switch_no, int state)
{
    uint32_t bit_mask = (uint32_t)1 << switch_no;
    if (state)
    {
        switches_state |= bit_mask;
    }
    else
    {
        switches_state &= ~bit_mask;
    }
    
    // apply state
    digitalWrite(PIN_SHIFT_REG_LATCH, LOW);
    shiftOut(PIN_SHIFT_REG_DATA, PIN_SHIFT_REG_CLOCK, MSBFIRST, (switches_state & 0x00FF0000) >> 16);
    shiftOut(PIN_SHIFT_REG_DATA, PIN_SHIFT_REG_CLOCK, MSBFIRST, (switches_state & 0x0000FF00) >> 8);
    shiftOut(PIN_SHIFT_REG_DATA, PIN_SHIFT_REG_CLOCK, MSBFIRST, (switches_state & 0x000000FF));
    digitalWrite(PIN_SHIFT_REG_LATCH, HIGH);
}

int get_switch_state(int switch_no)
{
    uint32_t bit_mask = (uint32_t)1 << switch_no;
    return switches_state & bit_mask ? 1 : 0;
}
