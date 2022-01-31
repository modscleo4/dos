#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_ADDR 0x100000

extern void restore_segment_selector(void);

void kernel_main(unsigned long int, unsigned long int);

#endif //KERNEL_H
