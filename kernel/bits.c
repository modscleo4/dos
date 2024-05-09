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

inline void outsb(uint16_t addr, uint8_t *buffer, uint32_t size) {
    asm volatile("rep outsb"
                 : "+S"(buffer),  "+c"(size)
                 : "d"(addr)
    );
}

inline void insb(uint16_t addr, uint8_t *buffer, uint32_t size) {
    asm volatile("rep insb"
                 : "+D"(buffer), "+c"(size)
                 : "d"(addr)
                 : "memory"
    );
}

inline void outsw(uint16_t addr, uint16_t *buffer, uint32_t size) {
    asm volatile("rep outsw"
                 : "+S"(buffer),  "+c"(size)
                 : "d"(addr)
    );
}

inline void insw(uint16_t addr, uint16_t *buffer, uint32_t size) {
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
    return (val << 8)
        | (val >> 8);
}

inline uint32_t switch_endian_32(uint32_t val) {
    return (val << 24)
        | ((val << 8) & 0x00FF0000)
        | ((val >> 8) & 0x0000FF00)
        | (val >> 24);
}

inline uint64_t switch_endian_64(uint64_t val) {
    return (val << 56)
        | ((val << 40) & 0x00FF000000000000)
        | ((val << 24) & 0x0000FF0000000000)
        | ((val << 8) & 0x000000FF00000000)
        | ((val >> 8) & 0x00000000FF000000)
        | ((val >> 24) & 0x0000000000FF0000)
        | ((val >> 40) & 0x000000000000FF00)
        | (val >> 56);
}

inline uint16_t popcnt16(uint16_t val) {
    uint16_t ret = 0;
    asm volatile("popcnt %1, %0;"
                 : "=r"(ret)
                 : "r"(val)
    );

    return ret;
}

inline uint32_t popcnt32(uint32_t val) {
    uint32_t ret = 0;
    asm volatile("popcnt %1, %0;"
                 : "=r"(ret)
                 : "r"(val)
    );

    return ret;
}

inline uint64_t popcnt64(uint64_t val) {
    uint64_t ret = 0;
#ifdef __x86_64__
    asm volatile("popcnt %1, %0;"
                 : "=r"(ret)
                 : "r"(val)
    );
#else
    ret = popcnt32(val & 0xFFFFFFFF) + popcnt32(val >> 32);
#endif

    return ret;
}
