#include "framebuffer.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/interrupts.h"
#include "../../modules/fonts/rasterized.h"

framebuffer_t fb0 = {
    .addr = (void *)0xB8000,
    .pitch = 160,
    .width = 80,
    .height = 25,
    .bpp = 16,
    .type = FRAMEBUFFER_TYPE_TEXT,
};

inline void framebuffer_rgb_draw_pixel(framebuffer_t *fb, int x, int y, uint32_t color) {
    uint32_t where = y * fb->pitch + x * fb->bpp / 8;

    ((uint8_t *)fb->addr)[where + 0] = ((uint8_t *)fb->backbuffer)[where + 0] = (color >> 0) & 0xFF;  // B
    ((uint8_t *)fb->addr)[where + 1] = ((uint8_t *)fb->backbuffer)[where + 1] = (color >> 8) & 0xFF;  // G
    ((uint8_t *)fb->addr)[where + 2] = ((uint8_t *)fb->backbuffer)[where + 2] = (color >> 16) & 0xFF; // R
}

inline void framebuffer_text_draw_char(framebuffer_t *fb, int x, int y, char c, uint8_t color) {
    ((uint16_t *)fb->addr)[y * fb->width + x] = (uint16_t)(c | (color << 8));
}

static void framebuffer_text_enable_cursor_caret(framebuffer_t *fb) {
    outb(SCREEN_CONTROL, 0x0A);
    outb(SCREEN_DATA, (inb(SCREEN_DATA) & 0xC0) | 12);

    outb(SCREEN_CONTROL, 0x0B);
    outb(SCREEN_DATA, (inb(SCREEN_DATA) & 0xE0) | 14);
}

static void framebuffer_text_disable_cursor_caret(framebuffer_t *fb) {
    outb(SCREEN_CONTROL, 0x0A);
    outb(SCREEN_DATA, 0x20);
}

void framebuffer_move_cursor_caret(framebuffer_t *fb, int x, int y) {
    //dbgprint("Moving cursor to %d, %d\n", x, y);
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT: {
            uint16_t pos = (y * fb->width + x);

            outb(SCREEN_CONTROL, 0x0F);
            outb(SCREEN_DATA, (uint8_t)(pos & 0xFF));
            outb(SCREEN_CONTROL, 0x0E);
            outb(SCREEN_DATA, (uint8_t)((pos >> 8) & 0xFF));

            break;
        }

        case FRAMEBUFFER_TYPE_RGB: {
            break;
        }
    }
}

static bool draw_caret = true;
void framebuffer_caret(framebuffer_t *fb) {
    /*if (fb->type == FRAMEBUFFER_TYPE_TEXT) {
        return;
    } else if (fb->type == FRAMEBUFFER_TYPE_RGB) {
        if (draw_caret) {
            framebuffer_draw_char(fb, curr_cursor_pos_x, curr_cursor_pos_y, '_', vgacolor);
        } else {
            framebuffer_draw_char(fb, curr_cursor_pos_x, curr_cursor_pos_y, ' ', vgacolor);
        }

        draw_caret = !draw_caret;
    }*/
}

void framebuffer_text_scroll(framebuffer_t *fb, void *backbuffer, int h) {
    memmove((void *)fb->addr, (void *)((uint8_t *)backbuffer + fb->pitch * h), fb->pitch * (fb->height - h));
    memset((void *)((uint8_t *)fb->addr + fb->pitch * (fb->height - h)), 0, fb->pitch * h);
}

void framebuffer_rgb_scroll(framebuffer_t *fb, void *backbuffer, int h) {
    memmove((void *)fb->addr, (void *)((uint8_t *)backbuffer + fb->pitch * h), fb->pitch * (fb->height - h));
    memset((void *)((uint8_t *)fb->addr + fb->pitch * (fb->height - h)), 0, fb->pitch * h);
}

static void framebuffer_flush(framebuffer_t *fb) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            // volatile uint8_t *video_memory = (volatile uint8_t *)fb->addr;

            // memcpy((void *)video_memory, (void *)video_buffer, fb->pitch * fb->height);
            /*for (int x = 0; x < video_buffer_width; x++) {
                for (int y = 0; y < video_buffer_height; y++) {
                    uint32_t where = y * fb->pitch + x * fb->bpp / 8;

                    uint32_t color = video_buffer[y * video_buffer_width + x];

                    video_memory[where + 0] = (color >> 0) & 0xFF;  // B
                    video_memory[where + 1] = (color >> 8) & 0xFF;  // G
                    video_memory[where + 2] = (color >> 16) & 0xFF; // R
                }
            }*/
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            // memcpy((void *)fb->addr, (void *)text_buffer, text_buffer_width * text_buffer_height * fb->bpp);

            break;
        }
    }

    framebuffer_move_cursor_caret(fb, 0, 0);
}

void framebuffer_setup(framebuffer_t *fb) {
    fb->backbuffer = calloc(fb->height, fb->pitch * fb->bpp / 8);
    if (fb->backbuffer == NULL) {
        dbgprint("Failed to allocate framebuffer backbuffer\n");
        return;
    }
}

void framebuffer_init(framebuffer_t *fb) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            //framebuffer_gotoxy(fb, 0, 0);
            //framebuffer_setcolor(fb, COLOR_BLACK << 4 | COLOR_GRAY);
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            framebuffer_text_disable_cursor_caret(fb);
            framebuffer_text_enable_cursor_caret(fb);

            //framebuffer_gotoxy(fb, 0, 0);
            //framebuffer_setcolor(fb, COLOR_BLACK << 4 | COLOR_GRAY);
            break;
        }
    }
}

int framebuffer_read(framebuffer_t *fb, uint8_t *buf, size_t count, size_t offset) {
    size_t size = fb->pitch * fb->height;
    if (offset >= size) {
        return 0;
    }

    if (offset + count > size) {
        count = size - offset;
    }

    memcpy((void *)buf, (void *)((uint8_t *)fb->backbuffer + offset), count);

    return count;
}

int framebuffer_write(framebuffer_t *fb, const uint8_t *buf, size_t count, size_t offset) {
    size_t size = fb->pitch * fb->height;
    if (offset >= size) {
        return 0;
    }

    if (offset + count > size) {
        count = size - offset;
    }

    memcpy((void *)((uint8_t *)fb->backbuffer + offset), (void *)buf, count);
    memcpy((void *)((uint8_t *)fb->addr + offset), (void *)buf, count);

    return count;
}
