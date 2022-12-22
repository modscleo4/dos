#ifndef SCREEN_H
#define SCREEN_H

#include <string.h>

#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

enum Colors {
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

enum ScreenRegisters {
    SCREEN_CONTROL = 0x3D4,
    SCREEN_DATA = 0x3D5
};

void video_init(int edx);

void setcolor(char c);

void clear_screen(void);

void gotoxy(int x, int y);

int screen_write(char c);

#endif //SCREEN_H
