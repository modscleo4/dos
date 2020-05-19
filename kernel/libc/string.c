#include <string.h>

void memcpy(void *source, void *dest, int n) {
    char *c_src = (char *) source;
    char *c_dest = (char *) dest;

    int i;
    for (i = 0; i < n; i++) {
        c_dest[i] = c_src[i];
    }
}
