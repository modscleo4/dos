#include "framebuffer.h"

#include <string.h>
#include "../screen.h"
#include "../../bits.h"

framebuffer_config *fb;
unsigned int curr_cursor_pos_x = 0;
unsigned int curr_cursor_pos_y = 0;
char color;

static inline int framebuffer_text_get_cur_pos(void) {
    return curr_cursor_pos_y * fb->width + curr_cursor_pos_x;
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

static void framebuffer_text_move_cursor_caret(void) {
    uint16_t pos = framebuffer_text_get_cur_pos();

    outb(SCREEN_CONTROL, 0x0F);
    outb(SCREEN_DATA, (uint8_t)(pos & 0xFF));
    outb(SCREEN_CONTROL, 0x0E);
    outb(SCREEN_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

static void framebuffer_text_handle_scroll(void) {
    volatile char *video_memory = (volatile char *)fb->addr;

    for (int i = 1; i < fb->height; i++) {
        memmove((void *)(2 * fb->width * (i - 1) + fb->addr), (void *)(2 * fb->width * i + fb->addr), fb->width * 2);
    }

    framebuffer_gotoxy(0, fb->height - 1);

    uint16_t pos = 2 * framebuffer_text_get_cur_pos();
    for (int i = 0; i < fb->width; i++) {
        video_memory[pos + 2 * i] = ' ';
        video_memory[pos + 2 * i + 1] = color;
    }
}

static int framebuffer_text_write(char c) {
    volatile char *video_memory = (volatile char *)fb->addr;

    switch (c) {
        case '\b':
            if (curr_cursor_pos_x == 0 && curr_cursor_pos_y == 0) {
                return 0;
            }

            curr_cursor_pos_x--;
            if (curr_cursor_pos_x == -1) {
                curr_cursor_pos_x = fb->width - 1;
                curr_cursor_pos_y--;
            }

            video_memory[2 * framebuffer_text_get_cur_pos()] = ' ';
            framebuffer_text_move_cursor_caret();
            return 0;

        case '\t': {
            int l = 8 - (curr_cursor_pos_x % 8);

            while (l--) {
                framebuffer_write(' ');
            }
            return 0;
        }

        case '\r':
            framebuffer_gotoxy(0, curr_cursor_pos_y);
            return 0;

        case '\n':
            if (curr_cursor_pos_y + 1 == fb->height) {
                framebuffer_text_handle_scroll();
            } else {
                framebuffer_gotoxy(0, curr_cursor_pos_y + 1);
            }

            return 0;

        default:
            break;
    }

    if (framebuffer_text_get_cur_pos() == fb->width * fb->height) {
        framebuffer_text_handle_scroll();
    }

    video_memory[2 * framebuffer_text_get_cur_pos()] = c;
    video_memory[2 * framebuffer_text_get_cur_pos() + 1] = color;

    curr_cursor_pos_x++;
    if (curr_cursor_pos_x == fb->width) {
        curr_cursor_pos_x = 0;
        curr_cursor_pos_y++;
    }

    framebuffer_text_move_cursor_caret();

    return 0;
}

void framebuffer_setup(framebuffer_config *config) {
    fb = config;
}

void framebuffer_init(void) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT: {
            framebuffer_text_disable_cursor_caret();
            framebuffer_text_enable_cursor_caret();

            framebuffer_gotoxy(0, 0);
            framebuffer_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
            break;
        }
    }
}

char framebuffer_getcolor(void) {
    return color;
}

void framebuffer_setcolor(char c) {
    color = c;
}

void framebuffer_clear(void) {
    framebuffer_gotoxy(0, 0);

    for (int i = 0; i < fb->width; i++) {
        for (int j = 0; j < fb->height; j++) {
            framebuffer_write(' ');
        }
    }

    framebuffer_gotoxy(0, 0);
}

void framebuffer_getxy(int *x, int *y) {
    *x = curr_cursor_pos_x;
    *y = curr_cursor_pos_y;
}

void framebuffer_gotoxy(const int x, const int y) {
    if (x < 0 || y < 0 || x >= fb->width || y >= fb->height) {
        return;
    }

    curr_cursor_pos_x = x;
    curr_cursor_pos_y = y;

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT: {
            framebuffer_text_move_cursor_caret();
            break;
        }
    }
}

int framebuffer_write(const char c) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_TEXT:
            return framebuffer_text_write(c);
    }

    return -1;
}

int framebuffer_write_str(const char *str) {
    int ret = 0;
    while (*str) {
        if (ret = framebuffer_write(*str++)) {
            return ret;
        }
    }

    return 0;
}
