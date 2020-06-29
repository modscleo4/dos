#include <ctype.h>

int toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return 'A' + c - 'a';
    }

    return c;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return 'a' + c - 'A';
    }

    return c;
}

