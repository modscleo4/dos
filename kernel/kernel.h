#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_ADDR 0x100000

extern char kernel_start_real_addr[];
extern char kernel_start_addr[];
extern char kernel_end_addr[];
extern char kernel_end_real_addr[];

extern void restore_segment_selector(void);

void kernel_main(unsigned long int magic, unsigned long int addr);

#endif //KERNEL_H
