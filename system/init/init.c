#include "init.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char str[1024];

    while (true) {
        printf("> ");
        gets(str);

        if (strlen(str) == 0) {
            continue;
        }

        char *cmd = str;

        if (strcmp(str, "ls") == 0) {
            printf("not implemented\n");
        } else if (strcmp(str, "exit") == 0) {
            break;
        } else if (strcmp(str, "div0") == 0) {
            printf("%f\n", 0.0F / .0F);
            printf("%f\n", 1.0F / .0F);
            printf("%f\n", -1.0F / .0F);
        } else if (strcmp(str, "panic") == 0) {
            asm volatile("hlt");
        } else {
            printf("%s: unknown command\n", cmd);
        }
    }

    return 0;
}
