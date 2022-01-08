#include "screen.h"

unsigned int curr_cursor_pos = 0;
char color;

void init_video() {
    char x = 0;
    char y = 0;
    asm("mov %%dl, %0;"
        : "=r"(x));

    asm("mov %%dh, %0;"
        : "=r"(y));

    gotoxy(x, y);
    setcolor(BLACK << 1 | GRAY);
}

void setcolor(char c) {
    color = c;
}

void clear_screen() {
    gotoxy(0, 0);
    color = 0x07;

    int i;
    for (i = 0; i < MAX_ROWS; i++) {
        int j;
        for (j = 0; j < MAX_COLS; j++) {
            screen_write(' ');
        }
    }

    gotoxy(0, 0);
}

void handle_scroll() {
    volatile char *video_memory = (volatile char *)VIDEO_ADDRESS;

    int i;
    for (i = 1; i < MAX_ROWS; i++) {
        memcpy((void *)(2 * MAX_COLS * (i - 1) + VIDEO_ADDRESS), (void *)(2 * MAX_COLS * i + VIDEO_ADDRESS),
               MAX_COLS * 2);
    }

    gotoxy(0, MAX_ROWS - 1);

    for (i = 0; i < MAX_COLS; i++) {
        video_memory[curr_cursor_pos + 2 * i] = ' ';
        video_memory[curr_cursor_pos + 2 * i + 1] = color;
    }
}

void gotoxy(int x, int y) {
    if (x < 0 || y < 0 || x >= MAX_COLS || y >= MAX_ROWS) {
        return;
    }

    curr_cursor_pos = 2 * (y * MAX_COLS + x);

    outb(CONTROL, 0x0F);
    outb(DATA, (unsigned char)(curr_cursor_pos / 2 & 0xFF));
    outb(CONTROL, 0x0E);
    outb(DATA, (unsigned char)((curr_cursor_pos / 2 >> 8) & 0xFF));
}

int screen_write(char c) {
    volatile char *video_memory = (volatile char *)VIDEO_ADDRESS;

    switch (c) {
        case '\b':
            if (curr_cursor_pos == 0) {
                return 0;
            }

            curr_cursor_pos -= 2;
            video_memory[curr_cursor_pos] = ' ';
            gotoxy((curr_cursor_pos / 2) % MAX_COLS, (curr_cursor_pos / 2) / MAX_COLS);
            return 0;

        case '\r':
            gotoxy(0, (curr_cursor_pos / 2) / MAX_COLS);
            return 0;

        case '\n':
            if ((curr_cursor_pos / 2) / MAX_COLS + 1 == MAX_ROWS) {
                handle_scroll();
            } else {
                gotoxy(0, (curr_cursor_pos / 2) / MAX_COLS + 1);
            }

            return 0;

        default:
            break;
    }

    if (curr_cursor_pos == 2 * MAX_ROWS * MAX_COLS) {
        handle_scroll();
    }

    video_memory[curr_cursor_pos++] = c;
    video_memory[curr_cursor_pos++] = color;
    outb(CONTROL, 0x0F);
    outb(DATA, (unsigned char)(curr_cursor_pos / 2 & 0xFF));
    outb(CONTROL, 0x0E);
    outb(DATA, (unsigned char)((curr_cursor_pos / 2 >> 8) & 0xFF));

    return 0;
}
