#ifndef KERNEL_SCREEN_H
#define KERNEL_SCREEN_H

enum ScreenColors {
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

enum ScreenMode {
    SCREEN_MODE_FRAMEBUFFER,
};

void screen_init(enum ScreenMode mode);

unsigned char screen_getcolor(void);

void screen_setcolor(unsigned char c);

void screen_clear(void);

void screen_getxy(int *x, int *y);

void screen_gotoxy(int x, int y);

int screen_write(const char c);

int screen_write_str(const char *str);

#endif // KERNEL_SCREEN_H
