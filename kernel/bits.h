#ifndef BITS_H
#define BITS_H

void outb(unsigned int, unsigned char);

unsigned char inb(unsigned int);

void outw(unsigned int, unsigned short int);

unsigned short int inw(unsigned int);

void io_wait(void);

#define DISABLE_BIT(reg, bit) reg &= ~(1 << bit)
#define ENABLE_BIT(reg, bit) reg |= (1 << bit)
#define TOGGLE_BIT(reg, bit) reg ^= (1 << bit)

#define DISABLE_BIT_INT(reg, bit) reg &= ~(bit)
#define ENABLE_BIT_INT(reg, bit) reg |= (bit)
#define TOGGLE_BIT_INT(reg, bit) reg ^= (bit)

#endif // BITS_H
