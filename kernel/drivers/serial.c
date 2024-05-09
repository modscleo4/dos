#include "serial.h"

#define DEBUG 0

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../debug.h"
#include "../bits.h"

static bool ports_enabled[8] = {false};

static uint8_t get_port_id(enum SerialPorts port) {
    switch (port) {
        case SERIAL_COM1:
            return 0;
        case SERIAL_COM2:
            return 1;
        case SERIAL_COM3:
            return 2;
        case SERIAL_COM4:
            return 3;
        case SERIAL_COM5:
            return 4;
        case SERIAL_COM6:
            return 5;
        case SERIAL_COM7:
            return 6;
        case SERIAL_COM8:
            return 7;
    }
}

bool is_serial_enabled(enum SerialPorts port) {
    return ports_enabled[get_port_id(port)];
}

bool serial_init(enum SerialPorts port, uint16_t baud_divisor) {
    if (port == 0) {
        return false;
    }

    if (is_serial_enabled(port)) {
        return false;
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

    while ((inb(port + SERIAL_REG_LINE_STATUS) & 0x20) == 0) {}

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(port + SERIAL_REG_DATA) != 0xAE) {
        return false;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + SERIAL_REG_MODEM_CONTROL, 0x0F);

    ports_enabled[get_port_id(port)] = true;

    return true;
}

bool serial_read(enum SerialPorts port, char *c) {
    if (port == 0) {
        return false;
    }

    if (!is_serial_enabled(port)) {
        return false;
    }

    while ((inb(port + SERIAL_REG_LINE_STATUS) & 0x01) == 0) { }

    *c = inb(port);

    return true;
}

bool serial_write(enum SerialPorts port, char c) {
    if (port == 0) {
        return false;
    }

    if (!is_serial_enabled(port)) {
        return false;
    }

    while ((inb(port + SERIAL_REG_LINE_STATUS) & 0x20) == 0) {}

    outb(port, c);

    return true;
}

bool serial_write_str(enum SerialPorts port, const char *str, ...) {
    va_list args;

    va_start(args, str);
    bool ret = serial_write_str_varargs(port, str, args);
    va_end(args);
    return ret;
}

bool serial_write_str_varargs(enum SerialPorts port, const char *str, va_list args) {
    if (port == 0) {
        return false;
    }

    if (!is_serial_enabled(port)) {
        return false;
    }

    char buffer[2048] = "";
    vsprintf(buffer, str, args);

    dbgprint("serial_write_str: %s\n", buffer);

    for (int i = 0; buffer[i]; i++) {
        serial_write(port, buffer[i]);
    }

    return true;
}
