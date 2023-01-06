#include <ctype.h>

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isalpha(int c) {
    return islower(c) || isupper(c);
}

int isblank(int c) {
    return c == ' ' || c == '\t';
}

int iscntrl(int c) {
    return (c >= 0 && c <= 31) || c == 127;
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isgraph(int c) {
    return c >= 33 && c <= 126;
}

int islower(int c) {
    return c >= 'a' && c <= 'z';
}

int isprint(int c) {
    return isgraph(c) || c == ' ';
}

int ispunct(int c) {
    return isgraph(c) && !isalnum(c);
}

int isspace(int c) {
    return isblank(c) || c >= 9 && c <= 13;
}

int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

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
