#include "bits.h"

void outb(uint16_t addr, uint8_t val) {
    asm volatile("outb %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

uint8_t inb(uint16_t addr) {
    uint8_t ret;
    asm volatile("inb %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void outw(uint16_t addr, uint16_t val) {
    asm volatile("outw %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

uint16_t inw(uint16_t addr) {
    uint16_t ret;
    asm volatile("inw %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void outl(uint16_t addr, uint32_t val) {
    asm volatile("outl %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

uint32_t inl(uint16_t addr) {
    uint32_t ret;
    asm volatile("inl %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

void insl(uint16_t addr, uint32_t *buffer, unsigned int quads) {
    for (int i = 0; i < quads; i++) {
        buffer[i] = inl(addr);
    }
}

void outsm(uint16_t addr, uint8_t *buffer, uint32_t size) {
    asm volatile("rep outsw"
                 : "+S"(buffer),  "+c"(size)
                 : "d"(addr)
    );
}

void insm(uint16_t addr, uint8_t *buffer, uint32_t size) {
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

uint16_t switch_endian_16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint32_t switch_endian_32(uint32_t val) {
    return (val << 24) | ((val << 8) & 0x00FF0000) | ((val >> 8) & 0x0000FF00) | (val >> 24);
}
