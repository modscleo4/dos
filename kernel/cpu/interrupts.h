#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

bool interrupts_was_enabled(void);

void interrupts_enable(void);

void interrupts_reenable(void);

void interrupts_disable(void);

#endif // INTERRUPT_H
