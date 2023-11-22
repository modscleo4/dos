#include "screen.h"

#include "video/framebuffer.h"

enum ScreenMode screen_mode = SCREEN_MODE_FRAMEBUFFER;

void screen_init(enum ScreenMode mode) {
    screen_mode = mode;
}

unsigned char screen_getcolor(void) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            return framebuffer_getcolor();
    }
}

void screen_setcolor(unsigned char c) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            framebuffer_setcolor(c);
    }
}

void screen_clear(void) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            framebuffer_clear();
    }
}

void screen_getxy(int *x, int *y) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            framebuffer_getxy(x, y);
    }
}

void screen_gotoxy(int x, int y) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            framebuffer_gotoxy(x, y);
    }
}

int screen_write(const char c) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            return framebuffer_write(c);
    }

    return -1;
}

int screen_write_str(const char *str) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            return framebuffer_write_str(str);
    }

    return -1;
}
