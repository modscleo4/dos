#include "screen.h"

unsigned int curr_cursor_pos = 0;

static inline void outb(unsigned int addr, unsigned char val) {
    asm volatile ("outb %0, %1;"
    :
    : "a" (val),
    "Nd" (addr));
}

static inline unsigned char inb(unsigned int addr) {
    unsigned char ret;
    asm volatile ("inb %1, %0;"
    : "=a" (ret)
    : "Nd" (addr));
    return ret;
}

void initvideo() {
    char x = 0;
    char y = 0;
    asm("mov %%dl, %0;"
    : "=r" (x));

    asm("mov %%dh, %0;"
    : "=r" (y));

    gotoxy(x, y);
}

void handle_scroll() {
    int i;
    for (i = 1; i < MAX_ROWS; i++) {
        memcpy((void *) (2 * MAX_COLS * i + VIDEO_ADDRESS), (void *) (2 * MAX_COLS * (i - 1) + VIDEO_ADDRESS),
               MAX_COLS * 2);
    }

    gotoxy(0, MAX_ROWS - 1);
}

void gotoxy(int x, int y) {
    if (x < 0 || y < 0 || x >= MAX_COLS || y >= MAX_ROWS) {
        return;
    }

    curr_cursor_pos = 2 * (y * MAX_COLS + x);

    outb(CONTROL, 0x0F);
    outb(DATA, (unsigned char) (curr_cursor_pos / 2 & 0xFF));
    outb(CONTROL, 0x0E);
    outb(DATA, (unsigned char) ((curr_cursor_pos / 2 >> 8) & 0xFF));
}

int screen_write(char c) {
    volatile char *video_memory = (volatile char *) VIDEO_ADDRESS;

    if (c == '\n') {
        if ((curr_cursor_pos / 2) / 80 + 1 == MAX_ROWS) {
            handle_scroll();
        } else {
            gotoxy(0, (curr_cursor_pos / 2) / 80 + 1);
        }

        return 0;
    }

    if (curr_cursor_pos == 2 * MAX_ROWS * MAX_COLS) {
        handle_scroll();
    }

    video_memory[curr_cursor_pos] = c;
    curr_cursor_pos += 2;
    outb(CONTROL, 0x0F);
    outb(DATA, (unsigned char) (curr_cursor_pos / 2 & 0xFF));
    outb(CONTROL, 0x0E);
    outb(DATA, (unsigned char) ((curr_cursor_pos / 2 >> 8) & 0xFF));

    return 0;
}
