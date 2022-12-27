#include "bits.h"

inline void outb(uint16_t addr, uint8_t val) {
    asm volatile("outb %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

inline uint8_t inb(uint16_t addr) {
    uint8_t ret;
    asm volatile("inb %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

inline void outw(uint16_t addr, uint16_t val) {
    asm volatile("outw %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

inline uint16_t inw(uint16_t addr) {
    uint16_t ret;
    asm volatile("inw %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

inline void outl(uint16_t addr, uint32_t val) {
    asm volatile("outl %0, %1;"
                 :
                 : "a"(val),
                   "Nd"(addr)
    );
}

inline uint32_t inl(uint16_t addr) {
    uint32_t ret;
    asm volatile("inl %1, %0;"
                 : "=a"(ret)
                 : "Nd"(addr)
    );
    return ret;
}

inline void insl(uint16_t addr, uint32_t *buffer, size_t quads) {
    for (size_t i = 0; i < quads; i++) {
        buffer[i] = inl(addr);
    }
}

inline void outsm(uint16_t addr, uint8_t *buffer, uint32_t size) {
    asm volatile("rep outsw"
                 : "+S"(buffer),  "+c"(size)
                 : "d"(addr)
    );
}

inline void insm(uint16_t addr, uint8_t *buffer, uint32_t size) {
    asm volatile("rep insw"
                 : "+D"(buffer), "+c"(size)
                 : "d"(addr)
                 : "memory"
    );
}

inline void io_wait(void) {
    asm volatile("outb %%al, $0x80"
                 :
                 : "a"(0));
}

inline uint16_t switch_endian_16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

inline uint32_t switch_endian_32(uint32_t val) {
    return (val << 24) | ((val << 8) & 0x00FF0000) | ((val >> 8) & 0x0000FF00) | (val >> 24);
}
