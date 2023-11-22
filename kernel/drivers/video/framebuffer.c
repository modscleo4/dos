#include "framebuffer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../screen.h"
#include "../../bits.h"
#include "../../modules/fonts/rasterized.h"

unsigned int text_buffer_width = 80;
unsigned int text_buffer_height = 25;
static char *text_buffer;
unsigned int video_buffer_width = 800;
unsigned int video_buffer_height = 600;
static uint8_t *video_buffer;

framebuffer_config *fb;
unsigned int curr_cursor_pos_x = 0;
unsigned int curr_cursor_pos_y = 0;
unsigned char vgacolor;

unsigned int pixelwidth = 1;
unsigned int pixelheight = 1;

static uint32_t vgacolor_to_rgb(uint8_t vgacolor) {
    uint32_t color = 0;

    switch (vgacolor) {
        case COLOR_BLACK:
            color = 0x000000;
            break;

        case COLOR_BLUE:
            color = 0x0000AA;
            break;

        case COLOR_GREEN:
            color = 0x00AA00;
            break;

        case COLOR_CYAN:
            color = 0x00AAAA;
            break;

        case COLOR_RED:
            color = 0xAA0000;
            break;

        case COLOR_PURPLE:
            color = 0xAA00AA;
            break;

        case COLOR_BROWN:
            color = 0xAA5500;
            break;

        case COLOR_GRAY:
            color = 0xAAAAAA;
            break;

        case COLOR_DARKGRAY:
            color = 0x555555;
            break;

        case COLOR_LIGHTBLUE:
            color = 0x5555FF;
            break;

        case COLOR_LIGHTGREEN:
            color = 0x55FF55;
            break;

        case COLOR_LIGHTCYAN:
            color = 0x55FFFF;
            break;

        case COLOR_LIGHTRED:
            color = 0xFF5555;
            break;

        case COLOR_LIGHTPURPLE:
            color = 0xFF55FF;
            break;

        case COLOR_YELLOW:
            color = 0xFFFF55;
            break;

        case COLOR_WHITE:
            color = 0xFFFFFF;
            break;
    }

    return color;
}

static inline void framebuffer_rgb_draw_pixel(int x, int y, uint32_t color) {
    volatile char *video_memory = (volatile char *)fb->addr;

    uint32_t where = y * fb->pitch + x * fb->bpp / 8;

    video_memory[where + 0] = (color >> 0) & 0xFF;  // B
    video_memory[where + 1] = (color >> 8) & 0xFF;  // G
    video_memory[where + 2] = (color >> 16) & 0xFF; // R
}

static void framebuffer_draw_char(int x, int y, char c, unsigned char color) {
    text_buffer[2 * (y * text_buffer_width + x)] = c;
    text_buffer[2 * (y * text_buffer_width + x) + 1] = color;

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            uint8_t *font_char = ascii[c];
            uint32_t color_on = vgacolor_to_rgb(color & 0xF);
            uint32_t color_off = vgacolor_to_rgb((color >> 4) & 0xF);

            for (int i = 0; i < pixelheight; i++) {
                for (int j = 0; j < pixelwidth; j++) {
                    bool bit = GET_BIT(font_char[i], j == pixelwidth - 1 ? 0 : pixelwidth - j - 1);
                    //framebuffer_rgb_draw_pixel(x * pixelwidth + j, y * pixelheight + i, bit ? color_on : color_off;
                    int _x = x * pixelwidth + j;
                    int _y = y * pixelheight + i;

                    //video_buffer[_y * video_buffer_width + _x] = bit ? color_on : color_off;
                    uint32_t where = _y * fb->pitch + _x * fb->bpp / 8;

                    video_buffer[where + 0] = (bit ? color_on : color_off >> 0) & 0xFF; // B
                    video_buffer[where + 1] = (bit ? color_on : color_off >> 8) & 0xFF; // G
                    video_buffer[where + 2] = (bit ? color_on : color_off >> 16) & 0xFF; // R
                }
            }
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT:
            break;
    }
}

static void framebuffer_text_enable_cursor_caret(void) {
    outb(SCREEN_CONTROL, 0x0A);
    outb(SCREEN_DATA, (inb(SCREEN_DATA) & 0xC0) | 12);

    outb(SCREEN_CONTROL, 0x0B);
    outb(SCREEN_DATA, (inb(SCREEN_DATA) & 0xE0) | 14);
}

static void framebuffer_text_disable_cursor_caret(void) {
    outb(SCREEN_CONTROL, 0x0A);
    outb(SCREEN_DATA, 0x20);
}

static void framebuffer_move_cursor_caret(void) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT: {
            uint16_t pos = (curr_cursor_pos_y * text_buffer_width + curr_cursor_pos_x);

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

static void framebuffer_flush(void) {
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

    framebuffer_move_cursor_caret();
}

static void framebuffer_handle_scroll(void) {
    //for (int i = 1; i < text_buffer_height; i++) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            memmove((void *)video_buffer, (void *)video_buffer + (fb->pitch * pixelheight), fb->pitch * (fb->height - pixelheight));

            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            memmove((void *)text_buffer, (void *)text_buffer + (2 * text_buffer_width), 2 * text_buffer_width * (text_buffer_height - 1));

            break;
        }
    }
    //}

    framebuffer_gotoxy(0, text_buffer_height - 1);

    uint16_t pos = 2 * (curr_cursor_pos_y * text_buffer_width + curr_cursor_pos_x);
    for (int i = 0; i < text_buffer_width; i++) {
        framebuffer_draw_char(i, curr_cursor_pos_y, ' ', vgacolor);
    }
}

static int framebuffer_text_write(char c) {
    switch (c) {
        case '\b':
            if (curr_cursor_pos_x == 0 && curr_cursor_pos_y == 0) {
                return 0;
            }

            curr_cursor_pos_x--;
            if (curr_cursor_pos_x == -1) {
                curr_cursor_pos_x = text_buffer_width - 1;
                curr_cursor_pos_y--;
            }

            framebuffer_draw_char(curr_cursor_pos_x, curr_cursor_pos_y, ' ', vgacolor);

            return 0;

        case '\t': {
            int l = 8 - (curr_cursor_pos_x % 8);

            while (l-- && curr_cursor_pos_x < text_buffer_width) {
                framebuffer_draw_char(curr_cursor_pos_x, curr_cursor_pos_y, ' ', vgacolor);
                curr_cursor_pos_x++;
            }

            return 0;
        }

        case '\r':
            framebuffer_gotoxy(0, curr_cursor_pos_y);
            return 0;

        case '\n':
            if (curr_cursor_pos_y + 1 == text_buffer_height) {
                framebuffer_handle_scroll();
            } else {
                framebuffer_gotoxy(0, curr_cursor_pos_y + 1);
            }

            return 0;

        default:
            break;
    }

    if ((curr_cursor_pos_y * text_buffer_width + curr_cursor_pos_x) == text_buffer_width * text_buffer_height) {
        framebuffer_handle_scroll();
    }

    framebuffer_draw_char(curr_cursor_pos_x, curr_cursor_pos_y, c, vgacolor);

    curr_cursor_pos_x++;
    if (curr_cursor_pos_x == text_buffer_width) {
        curr_cursor_pos_x = 0;
        curr_cursor_pos_y++;
    }

    return 0;
}

void framebuffer_setup(framebuffer_config *config) {
    fb = config;

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            pixelwidth = 9;
            pixelheight = 16;
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            pixelwidth = 1;
            pixelheight = 1;
            break;
        }
    }

    if (text_buffer) {
        unsigned int curr_x = curr_cursor_pos_x;
        unsigned int curr_y = curr_cursor_pos_y;

        //framebuffer_flush();

        //free(text_buffer);

        framebuffer_gotoxy(curr_x, curr_y);
    }

    text_buffer_width = fb->width / pixelwidth;
    text_buffer_height = fb->height / pixelheight;
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            text_buffer = (char *)malloc(text_buffer_width * text_buffer_height * 2);
            // video_buffer = (uint8_t *)calloc(fb->height, fb->pitch);
            video_buffer = (uint8_t *)fb->addr;
            video_buffer_width = fb->width;
            video_buffer_height = fb->height;
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            // Disable text_buffer for text mode
            text_buffer = (char *)fb->addr;
            video_buffer = NULL;
            video_buffer_width = 0;
            video_buffer_height = 0;
            break;
        }
    }
}

void framebuffer_init(void) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            framebuffer_gotoxy(0, 0);
            framebuffer_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
            break;
        }

        case FRAMEBUFFER_TYPE_TEXT: {
            framebuffer_text_disable_cursor_caret();
            framebuffer_text_enable_cursor_caret();

            framebuffer_gotoxy(0, 0);
            framebuffer_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
            break;
        }
    }
}

unsigned char framebuffer_getcolor(void) {
    return vgacolor;
}

void framebuffer_setcolor(unsigned char c) {
    vgacolor = c;
}

void framebuffer_clear(void) {
    for (int i = 0; i < text_buffer_width; i++) {
        for (int j = 0; j < text_buffer_height; j++) {
            framebuffer_draw_char(i, j, ' ', vgacolor);
        }
    }

    framebuffer_gotoxy(0, 0);

    framebuffer_flush();
}

void framebuffer_getxy(int *x, int *y) {
    *x = curr_cursor_pos_x;
    *y = curr_cursor_pos_y;
}

void framebuffer_gotoxy(int x, int y) {
    if (x < 0) {
        x += text_buffer_width;
    }

    if (y < 0) {
        y += text_buffer_height;
    }

    if (x >= text_buffer_width || y >= text_buffer_height) {
        return;
    }

    curr_cursor_pos_x = x;
    curr_cursor_pos_y = y;

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT: {
            framebuffer_move_cursor_caret();
            break;
        }
    }
}

int framebuffer_write(const char c) {
    int ret = framebuffer_text_write(c);

    framebuffer_flush();

    return ret;
}

int framebuffer_write_str(const char *str) {
    int ret = 0;
    while (*str) {
        if ((ret = framebuffer_text_write(*str++))) {
            framebuffer_flush();
            return ret;
        }
    }

    framebuffer_flush();

    return ret;
}
