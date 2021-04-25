#ifndef SCREEN_H
#define SCREEN_H

#include <string.h>

#include "../bits.h"

#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

enum Colors {
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    PURPLE,
    BROWN,
    GRAY,
    DARKGRAY,
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTPURPLE,
    YELLOW,
    WHITE
};

enum ScrrenRegisters {
    CONTROL = 0x3D4,
    DATA = 0x3D5
};

void init_video();

void gotoxy(int, int);

int screen_write(char);

#endif //SCREEN_H
