#ifndef TTY_H
#define TTY_H

#include <stdbool.h>
#include <stdint.h>
#include "video/framebuffer.h"

typedef struct tty_t {
    char *in_buffer;        /* input buffer */
    int in_buffer_size;     /* size of input buffer */
    int in_buffer_start;    /* last read index */
    int in_buffer_end;      /* last typed index */

    char *out_buffer;       /* output buffer */
    int out_buffer_size;    /* size of output buffer */
    int out_buffer_start;   /* last flushed index */
    int out_buffer_end;     /* last wrote index */

    int num;                /* tty number */
    bool canon;             /* canonical mode */
    bool echo;              /* echo mode */
    framebuffer_t *fb;      /* framebuffer to write to */

    uint16_t *text_buffer;  /* text buffer */
    int text_buffer_width;  /* width of text buffer */
    int text_buffer_height; /* height of text buffer */
    int text_buffer_pos_x;  /* current x position */
    int text_buffer_pos_y;  /* current y position */
    uint8_t text_color;     /* VGA text color */
    uint8_t char_width;     /* width of a character */
    uint8_t char_height;    /* height of a character */
} tty_t;

void tty_init(framebuffer_t *fb);

tty_t *tty_open(int tty_num);

int tty_switch(int tty_num);

int tty_get_current(void);

int tty_get_count(void);

int tty_chars_ready(tty_t *tty);

int tty_read_char(tty_t *tty, int scancode);

bool tty_canon_has_eol(tty_t *tty);

int tty_read(tty_t *tty, char *buf, int len);

int tty_write(tty_t *tty, const char *buf, int len);

int tty_flush(tty_t *tty);

int tty_getxy(tty_t *tty, int *x, int *y);

int tty_gotoxy(tty_t *tty, int x, int y);

int tty_clear(tty_t *tty);

int tty_getcolor(tty_t *tty, uint8_t *c);

int tty_setcolor(tty_t *tty, uint8_t c);

#endif // TTY_H
