#include "init.h"

extern void _init_stdio();

void init() {
    _init_stdio();
    printf("Ring3!\n");

    char str[1024];
    while (true) {
        printf("> ");
        gets(str);

        if (strcmp(str, "ls") == 0) {
            printf("not implemented\n");
        } else if (strcmp(str, "exit") == 0) {
            break;
        } else {
            printf("unknown command\n");
        }
    }
}

int main() {
    init();

    /*for (;;) {
        //
    }*/

    return 0;
}
