#include "keyboard.h"

unsigned char scancode;
int irq1_c = 0;

void init_keyboard() {
    irq_install_handler(1, keyboard_handler);
}

void keyboard_handler(struct registers *r) {
    irq1_c++;
    scancode = inb(0x60);
}

void wait_irq1() {
    while (irq1_c <= 0);
    irq1_c--;
}

unsigned char keyboard_read() {
    wait_irq1();
    if (scancode & 0x80) {
        return 0;
    } else {
        return kblayout[scancode];
    }
}
