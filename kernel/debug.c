#include "debug.h"

#include <stdio.h>

#define DEBUG

void dbgprint(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
#ifdef DEBUG
    vprintf(msg, args);
#endif
    va_end(args);
}

void hexdump(void *ptr, size_t n) {
    unsigned char *ptr_c = ptr;

    for (int i = 0; i < n; i++) {
        printf("%02x ", ptr_c[i]);
        if (i % 16 == 15 || i == n - 1) {
            if (i % 16 < 15) {
                for (int j = i % 16; j < 15; j++) {
                    printf("   ");
                }
            }

            printf("\t");
            for (int j = i - (i % 16); j <= i; j++) {
                if (ptr_c[j] >= 32 && ptr_c[j] <= 126) {
                    printf("%c", ptr_c[j]);
                } else {
                    printf(".");
                }
            }

            printf("\n");
        }
    }

    printf("\n");
}
