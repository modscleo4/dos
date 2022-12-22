#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct serial_device {
    uint16_t port;
    uint16_t baud_divisor;
} serial_device;

enum SerialPorts {
    SERIAL_COM1 = 0x3F8,
    SERIAL_COM2 = 0x2F8,
    SERIAL_COM3 = 0x3E8,
    SERIAL_COM4 = 0x2E8,
    SERIAL_COM5 = 0x5F8,
    SERIAL_COM6 = 0x4F8,
    SERIAL_COM7 = 0x5E8,
    SERIAL_COM8 = 0x4E8,
};

enum SerialRegisters {
    SERIAL_REG_DATA = 0,
    SERIAL_REG_INTERRUPT_ENABLE = 1,
    SERIAL_REG_BAUD_LOW = 0,
    SERIAL_REG_BAUD_HIGH = 1,
    SERIAL_REG_FIFO_CONTROL = 2,
    SERIAL_REG_LINE_CONTROL = 3,
    SERIAL_REG_MODEM_CONTROL = 4,
    SERIAL_REG_LINE_STATUS = 5,
    SERIAL_REG_MODEM_STATUS = 6,
    SERIAL_REG_SCRATCH = 7,
};

bool is_serial_enabled(uint16_t port);

serial_device *serial_init(uint16_t port, uint16_t baud_divisor);

bool serial_read(serial_device *device, char *c);

bool serial_write(serial_device *device, char c);

bool serial_write_str(serial_device *device, const char *str);

#endif // SERIAL_H
