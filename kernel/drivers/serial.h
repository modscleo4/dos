#ifndef SERIAL_H
#define SERIAL_H

enum SerialPorts {
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
    COM5 = 0x5F8,
    COM6 = 0x4F8,
    COM7 = 0x5E8,
    COM8 = 0x4E8,
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

int serial_init(unsigned int port, unsigned short int baud_divisor);

char serial_read(unsigned int port);

void serial_write(unsigned int port, char c);

void serial_write_str(unsigned int port, const char *str);

#endif // SERIAL_H
