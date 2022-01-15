#include "screen.h"

#include "../bits.h"

unsigned int curr_cursor_pos = 0;
char color;

void video_init(int edx) {
    char x = 0;
    char y = 0;

    x = (edx & 0x00FF);
    y = (edx & 0xFF00) >> 8;

    gotoxy(x, y);
    setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
}

void setcolor(char c) {
    color = c;
}

void clear_screen(void) {
    gotoxy(0, 0);
    color = 0x07;

    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            screen_write(' ');
        }
    }

    gotoxy(0, 0);
}

void handle_scroll(void) {
    volatile char *video_memory = (volatile char *)VIDEO_ADDRESS;

    for (int i = 1; i < MAX_ROWS; i++) {
        memcpy((void *)(2 * MAX_COLS * (i - 1) + VIDEO_ADDRESS), (void *)(2 * MAX_COLS * i + VIDEO_ADDRESS),
               MAX_COLS * 2);
    }

    gotoxy(0, MAX_ROWS - 1);

    for (int i = 0; i < MAX_COLS; i++) {
        video_memory[curr_cursor_pos + 2 * i] = ' ';
        video_memory[curr_cursor_pos + 2 * i + 1] = color;
    }
}

void move_cursor_caret(void) {
    outb(SCREEN_CONTROL, 0x0F);
    outb(SCREEN_DATA, (unsigned char)(curr_cursor_pos / 2 & 0xFF));
    outb(SCREEN_CONTROL, 0x0E);
    outb(SCREEN_DATA, (unsigned char)((curr_cursor_pos / 2 >> 8) & 0xFF));
}

void gotoxy(int x, int y) {
    if (x < 0 || y < 0 || x >= MAX_COLS || y >= MAX_ROWS) {
        return;
    }

    curr_cursor_pos = 2 * (y * MAX_COLS + x);

    move_cursor_caret();
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

        case '\t': {
            int l = 8 - ((curr_cursor_pos / 2) % 8);

            while (l--) {
                screen_write(' ');
            }
            return 0;
        }

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
    move_cursor_caret();

    return 0;
}
