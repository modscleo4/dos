#include "pic.h"

#include "../bits.h"

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2 + PIC_COMMAND, 0x20);
    }

    outb(PIC1 + PIC_COMMAND, 0x20);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t a1 = inb(PIC1 + PIC_DATA);
    uint8_t a2 = inb(PIC2 + PIC_DATA);

    outb(PIC1 + PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2 + PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1 + PIC_DATA, offset1);
    io_wait();
    outb(PIC2 + PIC_DATA, offset2);
    io_wait();
    outb(PIC1 + PIC_DATA, 4);
    io_wait();
    outb(PIC2 + PIC_DATA, 2);
    io_wait();

    outb(PIC1 + PIC_DATA, ICW4_8086);
    io_wait();
    outb(PIC2 + PIC_DATA, ICW4_8086);
    io_wait();

    outb(PIC1 + PIC_DATA, a1);
    outb(PIC2 + PIC_DATA, a2);
}
