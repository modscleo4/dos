#include "bits.h"

void outb(unsigned int addr, unsigned char val) {
    asm volatile("outb %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr));
}

unsigned char inb(unsigned int addr) {
    unsigned char ret;
    asm volatile("inb %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr));
    return ret;
}

void outw(unsigned int addr, unsigned short int val) {
    asm volatile("outw %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr));
}

unsigned short int inw(unsigned int addr) {
    unsigned short int ret;
    asm volatile("inw %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr));
    return ret;
}

void io_wait(void) {
    asm volatile("outb %%al, $0x80"
                 :
                 : "a"(0));
}
