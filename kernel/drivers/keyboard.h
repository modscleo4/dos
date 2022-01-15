#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/system.h"

void keyboard_init(void);

void keyboard_handler(struct registers *);

void wait_irq1(void);

char keyboard_read(void);

#endif //KEYBOARD_H
