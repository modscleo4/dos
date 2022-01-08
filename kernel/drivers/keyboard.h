#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/irq.h"
#include "../modules/kblayout/us.h"

void init_keyboard();

void keyboard_handler(struct registers *);

void wait_irq1();

char keyboard_read();

#endif //KEYBOARD_H
