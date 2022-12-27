#include "screen.h"

#include "video/framebuffer.h"

int screen_mode = SCREEN_MODE_FRAMEBUFFER;

void screen_init(int mode) {
    screen_mode = mode;
}

char screen_getcolor(void) {
    switch (screen_mode) {
        case SCREEN_MODE_FRAMEBUFFER:
            return framebuffer_getcolor();
    }
}

void screen_setcolor(char c) {
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

void screen_gotoxy(const int x, const int y) {
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
}

int screen_write_str(const char *str) {
    while (*str) {
        if (screen_write(*str++)) {
            return 1;
        }
    }

    return 0;
}
