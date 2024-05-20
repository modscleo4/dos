#ifndef KERNEL_FRAMEBUFFER_H
#define KERNEL_FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct framebuffer_t {
    void *addr;       /* physical address */
    uint32_t pitch;   /* number of bytes per line */
    uint32_t width;   /* width in pixels */
    uint32_t height;  /* height in pixels */
    uint8_t bpp;      /* bits per pixel */
    uint8_t type;     /* framebuffer type */

    void *backbuffer; /* backbuffer */
} framebuffer_t;

enum ScreenRegisters {
    SCREEN_CONTROL = 0x3D4,
    SCREEN_DATA = 0x3D5
};

enum FramebufferType {
    FRAMEBUFFER_TYPE_INDEXED = 0,
    FRAMEBUFFER_TYPE_RGB = 1,
    FRAMEBUFFER_TYPE_TEXT = 2,
};

enum ScreenColors {
    COLOR_BLACK = 0,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_PURPLE,
    COLOR_BROWN,
    COLOR_GRAY,
    COLOR_DARKGRAY,
    COLOR_LIGHTBLUE,
    COLOR_LIGHTGREEN,
    COLOR_LIGHTCYAN,
    COLOR_LIGHTRED,
    COLOR_LIGHTPURPLE,
    COLOR_YELLOW,
    COLOR_WHITE
};

extern framebuffer_t fb0;

void framebuffer_setup(framebuffer_t *fb);

void framebuffer_init(framebuffer_t *fb);

void framebuffer_caret(framebuffer_t *fb);

void framebuffer_text_scroll(framebuffer_t *fb, void *backbuffer, int h);

void framebuffer_rgb_scroll(framebuffer_t *fb, void *backbuffer, int h);

void framebuffer_move_cursor_caret(framebuffer_t *fb, int x, int y);

void framebuffer_rgb_draw_pixel(framebuffer_t *fb, int x, int y, uint32_t color);

void framebuffer_text_draw_char(framebuffer_t *fb, int x, int y, char c, uint8_t color);

int framebuffer_read(framebuffer_t *fb, uint8_t *buf, size_t count, size_t offset);

int framebuffer_write(framebuffer_t *fb, const uint8_t *buf, size_t count, size_t offset);

#endif // KERNEL_FRAMEBUFFER_H
