#include "serial.h"

#include <stdbool.h>
#include "../debug.h"
#include "../bits.h"

int serial_init(unsigned int port, unsigned short int baud_divisor) {
    if (port == 0) {
        return -1;
    }

    outb(port + SERIAL_REG_INTERRUPT_ENABLE, 0x00);
    outb(port + SERIAL_REG_LINE_CONTROL, 0x80);
    outb(port + SERIAL_REG_BAUD_LOW, (unsigned char)(baud_divisor & 0xFF));
    outb(port + SERIAL_REG_BAUD_HIGH, (unsigned char)((baud_divisor >> 8) & 0xFF));
    outb(port + SERIAL_REG_LINE_CONTROL, 0x03);
    outb(port + SERIAL_REG_FIFO_CONTROL, 0xC7);
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x0B);
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x1E);
    outb(port + SERIAL_REG_DATA, 0xAE);

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(port + SERIAL_REG_DATA) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x0F);
    return 0;
}

char serial_read(unsigned int port) {
    while (inb(port + SERIAL_REG_LINE_STATUS) & 0x01 == 0) { }

    return inb(port);
}

void serial_write(unsigned int port, char c) {
    while (inb(port + SERIAL_REG_LINE_STATUS) & 0x20 == 0) {}

    outb(port, c);
}

void serial_write_str(unsigned int port, const char *str) {
    while (*str) {
        serial_write(port, *str++);
    }
}
