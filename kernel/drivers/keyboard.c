#include "keyboard.h"

#define DEBUG 1

#include <string.h>
#include "../bits.h"
#include "../debug.h"
#include "../cpu/irq.h"
#include "../cpu/panic.h"
#include "../cpu/pic.h"
#include "../modules/kblayout/kb.h"

unsigned char scancode;
int irq_c = 0;
bool pressed[256];

static void _keyboard_reset(void) {
    char tmp = inb(KB_RESET_REGISTER);
    outb(KB_RESET_REGISTER, tmp | 0x80);
    outb(KB_RESET_REGISTER, tmp & 0x7F);
    inb(KB_DATA_REGISTER);
}

void keyboard_handler(registers *r, uint32_t int_no) {
    while (ISSET_BIT_INT(inb(KB_STATUS_REGISTER), 2)) {}
    irq_c++;
    scancode = inb(KB_DATA_REGISTER);

    if (!ISSET_BIT_INT(scancode, 0x80)) {
        pressed[scancode] = true;
    } else {
        pressed[DISABLE_BIT_INT(scancode, 0x80)] = false;
    }

    // Check for CTRL + ALT + DEL
    if (pressed[0x1d] && pressed[0x38] && pressed[0x53]) {
        panic("CTRL + ALT + DEL");
    }
}

void keyboard_init(void) {
    irq_c = 0;
    irq_install_handler(IRQ_KEYBOARD, keyboard_handler);
    _keyboard_reset();
    kblayout = kblayout_us;
    memset(pressed, 0, sizeof(pressed));
}

void keyboard_wait_irq(void) {
    while (irq_c <= 0) {}
    irq_c--;
}

char keyboard_read(void) {
    keyboard_wait_irq();
    return scancode;
}

void keyboard_clear_buffer(void) {
    char temp;
    do {
        temp = inb(KB_STATUS_REGISTER);
        if (ISSET_BIT(temp, 0)) {
            inb(KB_DATA_REGISTER);
        }
    } while (ISSET_BIT(temp, 1));
}
