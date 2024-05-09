#include "pic.h"

#define DEBUG 1

#include "../bits.h"
#include "../debug.h"

static inline uint8_t pic_read(enum PIC pic, enum PICRegister reg) {
    return inb(pic + reg);
}

static inline void pic_write(enum PIC pic, enum PICRegister reg, uint8_t data) {
    outb(pic + reg, data);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        pic_write(PIC2, PIC_COMMAND, 0x20);
    }

    pic_write(PIC1, PIC_COMMAND, 0x20);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    //uint8_t a1 = pic_read(PIC1, PIC_DATA);
    //uint8_t a2 = pic_read(PIC2, PIC_DATA);
    uint8_t a1 = 0;
    uint8_t a2 = 0;

    pic_write(PIC1, PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    pic_write(PIC2, PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    pic_write(PIC1, PIC_DATA, offset1);
    io_wait();
    pic_write(PIC2, PIC_DATA, offset2);
    io_wait();
    pic_write(PIC1, PIC_DATA, 4);
    io_wait();
    pic_write(PIC2, PIC_DATA, 2);
    io_wait();

    pic_write(PIC1, PIC_DATA, ICW4_8086);
    io_wait();
    pic_write(PIC2, PIC_DATA, ICW4_8086);
    io_wait();

    pic_write(PIC1, PIC_DATA, a1);
    pic_write(PIC2, PIC_DATA, a2);
}
