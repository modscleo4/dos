#ifndef BITS_H
#define BITS_H

#include <stddef.h>
#include <stdint.h>

void outb(uint16_t addr, uint8_t val);

uint8_t inb(uint16_t addr);

void outw(uint16_t addr, uint16_t val);

uint16_t inw(uint16_t addr);

void outl(uint16_t addr, uint32_t val);

uint32_t inl(uint16_t addr);

void insl(uint16_t addr, uint32_t *buffer, size_t quads);

void outsb(uint16_t addr, uint8_t *buffer, uint32_t size);

void insb(uint16_t addr, uint8_t *buffer, uint32_t size);

void outsw(uint16_t addr, uint16_t *buffer, uint32_t size);

void insw(uint16_t addr, uint16_t *buffer, uint32_t size);

void io_wait(void);

uint16_t switch_endian_16(uint16_t val);

uint32_t switch_endian_32(uint32_t val);

uint64_t switch_endian_64(uint64_t val);

uint16_t popcnt16(uint16_t val);

uint32_t popcnt32(uint32_t val);

uint64_t popcnt64(uint64_t val);

#define GET_BIT(reg, bit) (((reg) >> (bit)) & 1)
#define SET_BIT(reg, bit) ((reg) | (1 << bit))

#define DISABLE_BIT(reg, bit) ((reg) & ~(1 << bit))
#define ENABLE_BIT(reg, bit) ((reg) | (1 << bit))
#define TOGGLE_BIT(reg, bit) ((reg) ^ (1 << bit))
#define ISSET_BIT(reg, bit) (((reg) & (1 << bit)) != 0)

#define DISABLE_BIT_INT(reg, bit) ((reg) & ~(bit))
#define ENABLE_BIT_INT(reg, bit) ((reg) | (bit))
#define TOGGLE_BIT_INT(reg, bit) ((reg) ^ (bit))
#define ISSET_BIT_INT(reg, bit) (((reg) & (bit)) != 0)

#endif // BITS_H
