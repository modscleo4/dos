#include "bits.h"

void outb(unsigned int addr, unsigned char val) {
    asm volatile("outb %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

unsigned char inb(unsigned int addr) {
    unsigned char ret;
    asm volatile("inb %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void outw(unsigned int addr, unsigned short int val) {
    asm volatile("outw %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

unsigned short int inw(unsigned int addr) {
    unsigned short int ret;
    asm volatile("inw %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void outl(unsigned int addr, unsigned int val) {
    asm volatile("outl %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

unsigned int inl(unsigned int addr) {
    unsigned long int ret;
    asm volatile("inl %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void insl(unsigned int addr, unsigned int *buffer, int quads) {
    for (int i = 0; i < quads; i++) {
        buffer[i] = inl(addr);
    }
}

void outsm(unsigned int addr, unsigned char *buffer, unsigned long int size) {
    asm volatile("rep outsw"
                 : "+S"(buffer),  "+c"(size)
                 : "d"(addr)
    );
}

void insm(unsigned int addr, unsigned char *buffer, unsigned long int size) {
    asm volatile("rep insw"
                 : "+D"(buffer), "+c"(size)
                 : "d"(addr)
                 : "memory"
    );
}

void io_wait(void) {
    asm volatile("outb %%al, $0x80"
                 :
                 : "a"(0));
}
