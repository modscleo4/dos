#include "serial.h"

#define DEBUG 1

#include <stdbool.h>
#include <stdlib.h>
#include "../debug.h"
#include "../bits.h"

static bool ports_enabled[8];
static uint16_t ports[8];
static int last_port = 0;

bool is_serial_enabled(uint16_t port) {
    if (port == 0) {
        return false;
    }

    for (int i = 0; i < 8; i++) {
        if (ports[i] == port) {
            return ports_enabled[i];
        }
    }

    return false;
}

serial_device *serial_init(uint16_t port, uint16_t baud_divisor) {
    if (port == 0) {
        return NULL;
    }

    if (is_serial_enabled(port)) {
        return NULL;
    }

    outb(port + SERIAL_REG_INTERRUPT_ENABLE, 0x00);
    outb(port + SERIAL_REG_LINE_CONTROL, 0x80);
    outb(port + SERIAL_REG_BAUD_LOW, (uint8_t)(baud_divisor & 0xFF));
    outb(port + SERIAL_REG_BAUD_HIGH, (uint8_t)((baud_divisor >> 8) & 0xFF));
    outb(port + SERIAL_REG_LINE_CONTROL, 0x03);
    outb(port + SERIAL_REG_FIFO_CONTROL, 0xC7);
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x0B);
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x1E);
    outb(port + SERIAL_REG_DATA, 0xAE);

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(port + SERIAL_REG_DATA) != 0xAE) {
        return NULL;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x0F);

    serial_device *device = malloc(sizeof(serial_device));
    device->port = port;
    device->baud_divisor = baud_divisor;

    return device;
}

bool serial_read(serial_device *device, char *c) {
    if (!device) {
        return false;
    }

    while (inb(device->port + SERIAL_REG_LINE_STATUS) & 0x01 == 0) { }

    *c = inb(device->port);

    return true;
}

bool serial_write(serial_device *device, char c) {
    if (!device) {
        return false;
    }

    while (inb(device->port + SERIAL_REG_LINE_STATUS) & 0x20 == 0) {}

    outb(device->port, c);

    return true;
}

bool serial_write_str(serial_device *device, const char *str) {
    while (*str) {
        if (!serial_write(device, *str++)) {
            return false;
        }
    }

    return true;
}
