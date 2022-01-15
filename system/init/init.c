#include "init.h"

extern void _init_stdio();

void _main(void) {
    _init_stdio();

    syscall(0, main());
}

int main(void) {
    unsigned long int esp;
    char str[1024];

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
        } else if (strcmp(str, "panic") == 0) {
            asm("hlt");
        } else {
            printf("unknown command\n");
        }
    }

    return 0;
}
