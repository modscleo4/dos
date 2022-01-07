#ifndef KERNEL_FPU_H
#define KERNEL_FPU_H

#include <stddef.h>

void fpu_load_control_word(const unsigned short int);

void fpu_init(void);

#endif // KERNEL_FPU_H
