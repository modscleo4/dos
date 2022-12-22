#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0

enum PICRegister {
    PIC_COMMAND = 0x00,
    PIC_DATA = 0x01,
};

enum ICW1 {
    ICW1_ICW4 = 0x01,
    ICW1_SINGLE = 0x02,
    ICW1_INTERVAL4 = 0x04,
    ICW1_LEVEL = 0x08,
    ICW1_INIT = 0x10,
};

enum ICW4 {
    ICW4_8086 = 0x01,
    ICW4_AUTO = 0x02,
    ICW4_BUF_SLAVE = 0x08,
    ICW4_BUF_MASTER = 0x0C,
    ICW4_SFNM = 0x10,
};

void pic_send_eoi(unsigned char irq);

void pic_remap(uint8_t offset1, uint8_t offset2);

#endif //KERNEL_PIC_H
