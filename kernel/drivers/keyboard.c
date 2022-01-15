#include "keyboard.h"

#include "../bits.h"
#include "../cpu/irq.h"
#include "../cpu/pic.h"
#include "../modules/kblayout/us.h"

unsigned char scancode;
int irq1_c = 0;

void _keyboard_reset(void) {
    char tmp = inb(0x61);
    outb(0x61, tmp | 0x80);
    outb(0x61, tmp & 0x7F);
    inb(0x60);
}

void keyboard_init(void) {
    irq1_c = 0;
    irq_install_handler(IRQ_KEYBOARD, keyboard_handler);
    _keyboard_reset();
}

void keyboard_handler(struct registers *r) {
    while (inb(0x64) & 2) { }
    irq1_c++;
    scancode = inb(0x60);
}

void wait_irq1(void) {
    while (irq1_c <= 0) {}
    irq1_c--;
}

char keyboard_read(void) {
    wait_irq1();
    if (scancode & 0x80) {
        return -1;
    } else {
        return kblayout[scancode];
    }
}
