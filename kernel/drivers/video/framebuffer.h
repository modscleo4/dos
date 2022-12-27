#ifndef KERNEL_FRAMEBUFFER_H
#define KERNEL_FRAMEBUFFER_H

#include <stdint.h>

typedef struct framebuffer_config {
    uint8_t id;
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
    FRAMEBUFFER_TYPE_TEXT = 0,
    FRAMEBUFFER_TYPE_CGA = 1,
    FRAMEBUFFER_TYPE_EGA = 2,
    FRAMEBUFFER_TYPE_VGA = 3,
};

void framebuffer_init(framebuffer_config *config);

char framebuffer_getcolor(void);

void framebuffer_setcolor(char c);

void framebuffer_clear(void);

void framebuffer_getxy(int *x, int *y);

void framebuffer_gotoxy(const int x, const int y);

int framebuffer_write(const char c);

#endif // KERNEL_FRAMEBUFFER_H
