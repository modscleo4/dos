#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

typedef union gdt_entry {
    #pragma pack(push, 1)
    struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } entry;
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct {
        uint32_t limit_low : 16;
        uint32_t base_low : 24;
        uint8_t accessed : 1;
        uint8_t read_write : 1;             // readable for code, writable for data
        uint8_t conforming_expand_down : 1; // conforming for code, expand down for data
        uint8_t code : 1;                   // 1 for code, 0 for data
        uint8_t code_data_segment : 1;      // should be 1 for everything but TSS and LDT
        uint8_t DPL : 2;                    // privilege level
        uint8_t present : 1;
        uint8_t limit_high : 4;
        uint8_t available : 1; // only used in software; has no effect on hardware
        uint8_t long_mode : 1;
        uint8_t big : 1; // 32-bit opcodes for code, uint32_t stack for data
        uint8_t granularity : 1; // 1 to use 4k page addressing, 0 for byte addressing
        uint8_t base_high : 8;
    } bits;
    #pragma pack(pop)
} gdt_entry;

#pragma pack(push, 1)
typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} gdt_ptr;
#pragma pack(pop)

enum GDTAccess {
    GDT_ACCESS_PRESENT = 0x80,
    GDT_ACCESS_RING0 = 0x00,
    GDT_ACCESS_RING1 = 0x20,
    GDT_ACCESS_RING2 = 0x40,
    GDT_ACCESS_RING3 = 0x60,
    GDT_ACCESS_SEGMENT = 0x10,
    GDT_ACCESS_TSS = 0x00,
    GDT_ACCESS_EXECUTABLE = 0x08,
    GDT_ACCESS_DATA = 0x00,
    GDT_ACCESS_DIRECTION_CONFORMING = 0x04,
    GDT_ACCESS_READ_WRITE = 0x02,
    GDT_ACCESS_READ_ONLY = 0x00,
    GDT_ACCESS_ACCESSED = 0x01
};

extern void gdt_flush(gdt_ptr *ptr);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

void gdt_init(void);

#endif //KERNEL_GDT_H
