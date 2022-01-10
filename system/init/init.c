#include "init.h"

extern void _init_stdio();

char str[1024];

void init() {
    _init_stdio();

    while (true) {
        printf("> ");
        gets(str);

        if (strcmp(str, "ls") == 0) {
            printf("not implemented\n");
        } else if (strcmp(str, "exit") == 0) {
            break;
        } else if (strcmp(str, "div0") == 0) {
            printf("%f\n", 0.0F / .0F);
            printf("%f\n", 1.0F / .0F);
            printf("%f\n", -1.0F / .0F);
        } else if (strcmp(str, "exc") == 0) {
            asm("hlt");
        } else {
            printf("unknown command\n");
        }
    }
}

int main() {
    init();

    return 0;
}
