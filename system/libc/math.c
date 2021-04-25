#include <math.h>

int ipow(int base, int power) {
    int i;
    for (i = (power >= 0) ? 0 : power; i < (power >= 0) ? power : 0; i += (power >= 0) ? 1 : -1) {
        base *= base;
    }

    return base;
}
