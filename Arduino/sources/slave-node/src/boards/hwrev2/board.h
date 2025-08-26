#ifndef BOARD_H
#define BOARD_H

#include <inttypes.h>

#define PIN_CAN0_INT 5
#define PIN_CAN0_CS  9
#define PIN_SHIFT_REG_LATCH 2
#define PIN_SHIFT_REG_DATA  3
#define PIN_SHIFT_REG_CLOCK 7

void board_setup();

void set_switch_state(int switch_no, int state);
int get_switch_state(int switch_no);

#endif // BOARD_H
