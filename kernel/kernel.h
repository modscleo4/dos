#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_ADDR 0x100000

extern void restore_segment_selector(void);

void kernel_main(unsigned long int magic, unsigned long int addr);

#endif //KERNEL_H
