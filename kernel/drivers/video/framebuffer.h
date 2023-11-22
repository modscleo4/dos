#ifndef KERNEL_FRAMEBUFFER_H
#define KERNEL_FRAMEBUFFER_H

#include <stdint.h>

typedef struct framebuffer_config {
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type;
} framebuffer_config;

enum ScreenRegisters {
    SCREEN_CONTROL = 0x3D4,
    SCREEN_DATA = 0x3D5
};

enum FramebufferType {
    FRAMEBUFFER_TYPE_INDEXED = 0,
    FRAMEBUFFER_TYPE_RGB = 1,
    FRAMEBUFFER_TYPE_TEXT = 2,
};

void framebuffer_setup(framebuffer_config *config);

void framebuffer_init(void);

unsigned char framebuffer_getcolor(void);

void framebuffer_setcolor(unsigned char c);

void framebuffer_clear(void);

void framebuffer_getxy(int *x, int *y);

void framebuffer_gotoxy(int x, int y);

int framebuffer_write(const char c);

int framebuffer_write_str(const char *str);

#endif // KERNEL_FRAMEBUFFER_H
