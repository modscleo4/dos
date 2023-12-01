#include "interrupts.h"

static bool was_interrupts_enabled = false;

bool interrupts_was_enabled(void) {
    return was_interrupts_enabled;
}

void interrupts_enable(void) {
    asm volatile("sti");
    was_interrupts_enabled = true;
}

void interrupts_reenable(void) {
    if (was_interrupts_enabled) {
        asm volatile("sti");
    }
}

void interrupts_disable(void) {
    asm volatile("cli");
}
