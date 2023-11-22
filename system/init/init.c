#include "init.h"

#include <stdbool.h>
#include <stdint.h>
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
            printf("%f\n", -1.0F);
        } else if (strcmp(str, "panic") == 0) {
            asm volatile("hlt");
        } else if (strncmp(str, "dns ", 4) == 0) {
            char *domain = str + 4;
            uint8_t ip[4];

            if (strlen(domain) == 0) {
                printf("dns: missing domain\n");
                continue;
            }

            if (syscall(5, 0, "A", domain, ip)) {
                printf("dns: IPv4 for %s is %d.%d.%d.%d\n", domain, ip[0], ip[1], ip[2], ip[3]);
            } else {
                printf("dns: failed to resolve %s\n", domain);
            }
        } else {
            printf("%s: unknown command\n", cmd);
        }
    }

    return 0;
}
