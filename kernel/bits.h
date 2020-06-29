#ifndef BITS_H
#define BITS_H

void outb(unsigned int, unsigned char);

unsigned char inb(unsigned int);

void outw(unsigned int, unsigned short int);

unsigned short int inw(unsigned int);

void io_wait(void);

#endif // BITS_H
