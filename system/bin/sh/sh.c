#include "sh.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("sh started with %d arguments, UID=%ld, GID=%ld\n", argc, getuid(), getgid());
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    char str[1024];
    while (true) {
        printf("> ");
        fgets(str, 1024, stdin);

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
        } else if (strcmp(str, "gpf") == 0) {
            asm volatile("hlt");
        } else if (strcmp(str, "panic") == 0) {
            syscall(127, "PANIC Test from ring3\n");
        } else if (strncmp(str, "while", 5) == 0) {
            long int i = 0;
            while (i < 0x7FFFFFFE) {
                i++;
            }
        } else if (strncmp(str, "nyx", 3) == 0) {
            printf("The moment man devoured the fruit of knowledge, he sealed his fate...\n");
            printf("Entrusting his future to the cards, he clings to a dim hope.\n");
            printf("Yes, \n");
            printf("\n");
            printf("The arcana is the means by which all is revealed... Attaining one's dream requires a stern will and unfailing determination.\n");
            printf("The arcana is the means by which all is revealed... The silent voice within one's heart whispers the most profound wisdom.\n");
            printf("The arcana is the means by which all is revealed... Celebrate life's grandeur... its brilliance... its magnificence...\n");
            printf("The arcana is the means by which all is revealed... Only Courage in the face of doubt can lead one to the answer...\n");
            printf("The arcana is the means by which all is revealed... It is indeed a precious gift to understand the forces that guides oneself...\n");
            printf("The arcana is the means by which all is revealed... There is both joy and wonder in coming to understand another...\n");
            printf("The arcana is the means by which all is revealed... One of life's greatest blessings is the freedom to pursue one's goals...\n");
            printf("The arcana is the means by which all is revealed... To find the one true path, one must seek guidance amidst uncertainty...\n");
            printf("The arcana is the means by which all is revealed... It requires great courage to look at oneself honestly, and forge one's own path...\n");
            printf("The arcana is the means by which all is revealed... Alongside time exists fate, the bearer of cruelty.\n");
            printf("The arcana is the means by which all is revealed... Only with Strength can one endure suffering and torment.\n");
            printf("The arcana is the means by which all is revealed... In the face of disaster lies the opportunity for renewal.\n");
            printf("\n");
            printf("The moment man devoured the fruit of knowledge, he sealed his fate...\n");
            printf("Entrusting his future to the cards, he clings to a dim hope.\n");
            printf("Yet, The arcana is the means by which all is revealed...\n");
            printf("Beyond the beaten path lies the absolute end.\n");
            printf("It matters not who you are... Death awaits you.\n");
        } else if (strncmp(str, "fork", 4) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                printf("I am the child, PID is %ld, parent PID is %ld\n", getpid(), getppid());
            } else {
                printf("I am the parent, PID is %ld\n", pid);
            }
        } else if (strncmp(str, "exec ", 5) == 0) {
            char *exe = str + 5;
            char *argv[] = {NULL};
            char *envp[] = {NULL};

            int r;
            if ((r = execve(exe, argv, envp))) {
                printf("exec failed: %d\n", r);
            }
        } else {
            printf("%s: unknown command\n", cmd);
        }
    }

    return 0;
}
