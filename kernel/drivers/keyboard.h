#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/system.h"

enum KeyboardRegisters {
    KB_DATA_REGISTER = 0x60,
    KB_RESET_REGISTER = 0x61,
    KB_STATUS_REGISTER = 0x64,
    KB_CTRL_REGISTER = 0x64
};

#define KB_RESET 0xFE

void keyboard_init(void);

void keyboard_wait_irq(void);

char keyboard_read(void);

void keyboard_clear_buffer(void);

#endif //KEYBOARD_H
