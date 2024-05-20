#include "tty.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "keyboard.h"
#include "video/framebuffer.h"
#include "../bits.h"
#include "../cpu/interrupts.h"
#include "../modules/kblayout/kb.h"
#include "../modules/fonts/rasterized.h"

static tty_t *ttys;
static int ttys_count = 4;
static int current_tty = -1;

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

static inline void tty_draw_char_framebuffer(tty_t *tty, int x, int y, char c, uint8_t color) {
    switch (tty->fb->type) {
        case FRAMEBUFFER_TYPE_RGB: {
            const uint8_t *font_char = ascii[c];
            uint32_t color_on = vgacolor_to_rgb(color & 0xF);
            uint32_t color_off = vgacolor_to_rgb((color >> 4) & 0xF);

            for (int i = 0; i < tty->char_height; i++) {
                for (int j = 0; j < tty->char_width; j++) {
                    bool bit = GET_BIT(font_char[i], j == tty->char_width - 1 ? 0 : tty->char_width - j - 1);
                    framebuffer_rgb_draw_pixel(tty->fb, x * tty->char_width + j, y * tty->char_height + i, bit ? color_on : color_off);
                }
            }

            break;
        }

        case FRAMEBUFFER_TYPE_TEXT:
            framebuffer_text_draw_char(tty->fb, x, y, c, color);
            break;
    }
}

static void tty_flush_text_buffer(tty_t *tty) {
    for (int i = 0; i < tty->text_buffer_width; i++) {
        for (int j = 0; j < tty->text_buffer_height; j++) {
            uint16_t c = tty->text_buffer[j * tty->text_buffer_width + i];
            char ch = c & 0xFF;
            uint8_t color = c >> 8;

            tty_draw_char_framebuffer(tty, i, j, ch, color);
        }
    }
}

static void tty_handle_scroll(tty_t *tty) {
    uint8_t *buf = (uint8_t *)tty->text_buffer;
    memmove(buf, buf + sizeof(uint16_t) * tty->text_buffer_width, sizeof(uint16_t) * tty->text_buffer_width * (tty->text_buffer_height - 1));
    memset(buf + sizeof(uint16_t) * tty->text_buffer_width * (tty->text_buffer_height - 1), 0, sizeof(uint16_t) * tty->text_buffer_width);
    switch (tty->fb->type) {
        case FRAMEBUFFER_TYPE_RGB:
            //tty_flush_text_buffer(tty);
            framebuffer_rgb_scroll(tty->fb, tty->fb->addr, tty->char_height);
            break;

        case FRAMEBUFFER_TYPE_TEXT:
            framebuffer_text_scroll(tty->fb, tty->fb->addr, 1);
            break;
    }

    tty_gotoxy(tty, 0, tty->text_buffer_height - 1);
}

static inline void tty_write_char_at(tty_t *tty, int x, int y, char c, uint8_t color) {
    tty->text_buffer[y * tty->text_buffer_width + x] = (color << 8) | c;
    tty_draw_char_framebuffer(tty, x, y, c, color);
}

static void tty_write_char(tty_t *tty, char c, uint8_t color) {
    switch (c) {
        case '\b':
            if (tty->text_buffer_pos_x == 0 && tty->text_buffer_pos_y == 0) {
                return;
            }

            tty->text_buffer_pos_x--;
            if (tty->text_buffer_pos_x == -1) {
                tty->text_buffer_pos_x = tty->text_buffer_width - 1;
                tty->text_buffer_pos_y--;
            }

            tty_write_char_at(tty, tty->text_buffer_pos_x, tty->text_buffer_pos_y, ' ', tty->text_color);

            framebuffer_move_cursor_caret(tty->fb, tty->text_buffer_pos_x, tty->text_buffer_pos_y);

            return;

        case '\t': {
            int l = 8 - (tty->text_buffer_pos_x % 8);

            while (l-- && tty->text_buffer_pos_x < tty->text_buffer_width) {
                tty_write_char_at(tty, tty->text_buffer_pos_x, tty->text_buffer_pos_y, ' ', tty->text_color);
                tty->text_buffer_pos_x++;
            }

            framebuffer_move_cursor_caret(tty->fb, tty->text_buffer_pos_x, tty->text_buffer_pos_y);

            return;
        }

        case '\r':
            tty_gotoxy(tty, 0, tty->text_buffer_pos_y);
            return;

        case '\n':
            if (tty->text_buffer_pos_y + 1 == tty->text_buffer_height) {
                tty_handle_scroll(tty);
            } else {
                tty_gotoxy(tty, 0, tty->text_buffer_pos_y + 1);
            }

            return;

        default:
            break;
    }

    if ((tty->text_buffer_pos_y * tty->text_buffer_width + tty->text_buffer_pos_x) == tty->text_buffer_width * tty->text_buffer_height) {
        tty_handle_scroll(tty);
    }

    tty_write_char_at(tty, tty->text_buffer_pos_x, tty->text_buffer_pos_y, c, tty->text_color);

    tty->text_buffer_pos_x++;
    if (tty->text_buffer_pos_x == tty->text_buffer_width) {
        tty->text_buffer_pos_x = 0;
        tty->text_buffer_pos_y++;
    }

    framebuffer_move_cursor_caret(tty->fb, tty->text_buffer_pos_x, tty->text_buffer_pos_y);
}

void tty_init(framebuffer_t *fb) {
    ttys = (tty_t *)calloc(ttys_count, sizeof(tty_t));

    for (int i = 0; i < 4; i++) {
        tty_t *tty = &ttys[i];

        tty->in_buffer_size  = 1024;
        tty->in_buffer       = (char *)malloc(tty->in_buffer_size);
        tty->in_buffer_start = 0;
        tty->in_buffer_end   = 0;

        tty->out_buffer_size  = 1024;
        tty->out_buffer       = (char *)malloc(tty->out_buffer_size);
        tty->out_buffer_start = 0;
        tty->out_buffer_end   = 0;

        tty->num   = i;
        tty->canon = true;
        tty->echo  = true;
        tty->fb    = fb;

        if (fb->type == FRAMEBUFFER_TYPE_TEXT) {
            tty->char_width  = 1;
            tty->char_height = 1;
        } else if (fb->type == FRAMEBUFFER_TYPE_RGB) {
            tty->char_width  = 9;
            tty->char_height = 16;
        }

        tty->text_buffer_width  = fb->width / tty->char_width;
        tty->text_buffer_height = fb->height / tty->char_height;
        tty->text_buffer_pos_x  = 0;
        tty->text_buffer_pos_y  = 0;
        tty->text_color         = COLOR_BLACK << 4 | COLOR_GRAY;
        tty->text_buffer        = (uint16_t *)malloc(tty->text_buffer_width * tty->text_buffer_height * sizeof(uint16_t));
    }

    current_tty = 0;
}

tty_t *tty_open(int tty_num) {
    if (tty_num < 0 || tty_num >= ttys_count) {
        return NULL;
    }

    return &ttys[tty_num];
}

int tty_switch(int tty_num) {
    if (tty_num < 0 || tty_num >= ttys_count) {
        return -ENODEV;
    }

    current_tty = tty_num;

    return 0;
}

int tty_get_current(void) {
    return current_tty;
}

int tty_get_count(void) {
    return ttys_count;
}

int tty_chars_ready(tty_t *tty) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    int end = tty->in_buffer_end;
    if (end < tty->in_buffer_start) {
        end += tty->in_buffer_size;
    }

    return end - tty->in_buffer_start;
}

int tty_read_char(tty_t *tty, int scancode) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    if (tty->in_buffer_end == tty->in_buffer_size) {
        return -ENOMEM;
    }

    char c = kblayout[scancode];

    if (tty->canon && c == '\b') {
        if (tty->in_buffer_end > tty->in_buffer_start) {
            tty->in_buffer_end--;
        } else {
            // ignore backspace if buffer is empty
            return 0;
        }
    } else {
        tty->in_buffer[tty->in_buffer_end++] = c;
    }

    if (tty->echo) {
        tty_write(tty, &c, 1);
    }

    return 0;
}

bool tty_canon_has_eol(tty_t *tty) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return false;
    }

    for (int i = tty->in_buffer_start; i != tty->in_buffer_end; i++) {
        if (i == tty->in_buffer_size) {
            i = 0;
        }

        if (tty->in_buffer[i] == '\n') {
            return true;
        }
    }

    return false;
}

int tty_read(tty_t *tty, char *buf, int len) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    size_t r = 0;
    for (; len && tty->in_buffer_start != tty->in_buffer_end; tty->in_buffer_start++) {
        if (tty->in_buffer_start == tty->in_buffer_size) {
            tty->in_buffer_start = 0;
        }

        if (tty->in_buffer_start == tty->in_buffer_end) {
            break;
        }

        buf[r] = tty->in_buffer[tty->in_buffer_start];
        len--;

        r++;
    }

    return r;
}

int tty_write(tty_t *tty, const char *buf, int len) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    size_t w = 0;
    for (; len; tty->out_buffer_end++) {
        if (tty->out_buffer_end == tty->out_buffer_size) {
            tty_flush(tty);
        }

        tty->out_buffer[tty->out_buffer_end] = buf[w];
        len--;

        w++;
    }

    tty_flush(tty);

    return w;
}

int tty_flush(tty_t *tty) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    for (int i = tty->out_buffer_start; i != tty->out_buffer_end; i++) {
        if (i == tty->out_buffer_size) {
            i = 0;
        }

        tty_write_char(tty, tty->out_buffer[i], tty->text_color);
    }

    tty->out_buffer_start = 0;
    tty->out_buffer_end = 0;

    return 0;
}

int tty_getcolor(tty_t *tty, uint8_t *c) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    *c = tty->text_color;

    return 0;
}

int tty_setcolor(tty_t *tty, uint8_t c) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    tty->text_color = c;

    return 0;
}

int tty_clear(tty_t *tty) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    memsetw(tty->text_buffer, tty->text_color << 8 | ' ', tty->text_buffer_width * tty->text_buffer_height);
    tty_flush_text_buffer(tty);

    tty_gotoxy(tty, 0, 0);

    return 0;
}

int tty_getxy(tty_t *tty, int *x, int *y) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    *x = tty->text_buffer_pos_x;
    *y = tty->text_buffer_pos_y;

    return 0;
}

int tty_gotoxy(tty_t *tty, int x, int y) {
    if (tty == NULL) {
        tty = &ttys[current_tty];
    }

    if (!tty) {
        return -ENODEV;
    }

    if (x < 0) {
        x += tty->text_buffer_width;
    }

    if (y < 0) {
        y += tty->text_buffer_height;
    }

    if (x >= tty->text_buffer_width || y >= tty->text_buffer_height) {
        return -EINVAL;
    }

    tty->text_buffer_pos_x = x;
    tty->text_buffer_pos_y = y;

    framebuffer_move_cursor_caret(tty->fb, x, y);

    return 0;
}
