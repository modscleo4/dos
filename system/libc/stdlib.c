#include <stdlib.h>

#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern void _init_stdio(void);
extern int main(int argc, char *argv[]);

volatile void _start(int argc, char *argv[], char *envp[]) {
    _init_stdio();

    exit(main(argc, argv));
}

static int min(int a, int b) {
    return a < b ? a : b;
}

static int max(int a, int b) {
    return a > b ? a : b;
}

static const char numbase[] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

float atof(const char *str) {
    return (float)strtod(str, NULL);
}

int atoi(const char *str) {
    return (int)atol(str);
}

char *htoa(short int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *itoa(int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *ltoa(long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    if (value < 0) {
        if (base == 10) {
            *ptr++ = '-';
        } else {
            value = -value;
        }
    }

    low = ptr;

    do {
        *ptr++ = numbase[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *hutoa(unsigned short int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *utoa(unsigned int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *lutoa(unsigned long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    low = ptr;

    do {
        *ptr++ = numbase[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *ftoa(float value, char *str, int precision) {
    if (isnanf(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinff(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    return lftoa((double)value, str, precision);
}

char *lftoa(double value, char *str, int precision) {
    if (isnanl(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinfl(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    ltoa((long int)value, str, 10);
    int i = strlen(str);
    if (precision > 0) {
        str[i] = '.';

        value -= (long int)value;
        value *= powl(10, precision);
        ltoa((long int)value, str + i + 1, 10);

        int j = strlen(str + i + 1);
        while (j < precision) {
            str[i + j + 1] = '0';
            str[i + j + 2] = 0;
            j++;
        }
    }

    return str;
}

long int atol(const char *str) {
    return strtol(str, NULL, 10);
}

double strtod(const char *str, char **endptr) {
    return 0.0F;
}

long int strtol(const char *str, char **endptr, int base) {
    if (base < 0 || base == 1 || base > 36) {
        if (endptr) {
            *endptr = (char *)str;
        }

        return 0;
    }

    while (isspace(*str)) {
        str++;
    }

    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    if (base == 0) {
        if (*str == '0') {
            if (str[1] == 'x' || str[1] == 'X') {
                base = 16;
                str += 2;
            } else {
                base = 8;
                str++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
            str += 2;
        }
    }

    if (!*str) {
        return 0;
    }

    long int value = 0;
    while (*str) {
        int digit = 0;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            return 0;
        }

        if (digit >= base) {
            return 0;
        }

        value = value * base + digit;
        str++;
    }

    if (endptr) {
        *endptr = (char *)str;
    }

    return value * sign;
}

unsigned long int strtoul(const char *str, char **endptr, int base) {
    return 0;
}

void *calloc(size_t num, size_t size) {
    return NULL;
}

void free(void *ptr) {
    //
}

static uintptr_t last_malloc_addr = 0xC00000;
void *malloc(size_t size) {
    uintptr_t addr = last_malloc_addr;
    last_malloc_addr += size;
    return (void *)addr;
}

void *realloc(void *ptr, size_t size) {
    return NULL;
}

void abort(void) {
    //
}

int atexit(void (*func)(void)) {
    return 0;
}

void exit(int status) {
    syscall(60, status);
}

char *getenv(const char *name) {
    return NULL;
}

int system(const char *command) {
    //floppy_load_file(command);

    return 0;
}

void swap(void *a, void *b, size_t size) {
    char tmp[size];
    memcpy(tmp, a, size);

    memcpy(a, b, size);
    memcpy(b, tmp, size);
}

/**
 * Binary Search implementation
 *
 * Assume that the array is sorted in ascending order.
 * 1. Compare x with the middle element.
 * 2. If x matches with middle element, we return the mid index.
 * 3. Else if x is greater than the mid element, then x can only lie in right half subarray after the mid element.
 *  So we recur for right half.
 * 4. Else (x is smaller) recur for the left half.
 * Go to step 1 while start <= end.
 * 5. We reach here if element was not present in array.
 */
void *bsearch(const void *key, const void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    size_t start = 0;
    size_t end = num;

    while (start < end) {
        size_t mid = (start + end) / 2;
        int cmp = compar(key, base + mid * size);
        if (cmp == 0) {
            return (void *)base + mid * size;
        } else if (cmp < 0) {
            end = mid;
        } else {
            start = mid + 1;
        }
    }

    return NULL;
}

/**
 * Quick Sort implementation in-place
 *
 * 1. Choose a pivot element from the list. We can choose the first element as the pivot element for simplicity.
 * 2. Reorder the list so that all elements with values less than the pivot element come before the pivot element,
 *   while all elements with values greater than the pivot element come after it (equal values can go either way).
 *  After this partitioning, the pivot element is in its final position. This is called the partition operation.
 * 3. Recursively apply the above steps to the sub-list of elements with smaller values and separately the sub-list
 *  of elements with greater values.
 *
 * Example:
 * | 5 | 3 | 7 | 6 | 2 | 9 | 1 | 4 | 8 |
 *   ^
 *  pivot
 *
 * After partitioning:
 * | 3 | 2 | 1 | 4 | 5 | 7 | 6 | 9 | 8 |
 */
static void qsort_inplace(void *base, size_t num, size_t size, int (*compar)(const void *, const void *), size_t start, size_t end) {
    if (end - start <= 1) {
        return;
    }

    // Partition
    // Move all elements smaller than the pivot to the left, and all greater than the pivot to the right
    size_t pivot_index = end - 1;

    size_t i = start - 1;
    for (size_t j = start; j < end - 1; j++) {
        int cmp = compar(base + j * size, base + pivot_index * size);
        if (cmp < 0) { // Smaller
            i++;
            swap(base + i * size, base + j * size, size);
        }
    }
    swap(base + (i + 1) * size, base + pivot_index * size, size);
    pivot_index = i + 1;

    qsort_inplace(base, num, size, compar, start, pivot_index);
    qsort_inplace(base, num, size, compar, pivot_index + 1, end);
}

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    qsort_inplace(base, num, size, compar, 0, num);
}

div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;

    if (numer >= 0 && result.rem < 0) {
        result.quot++;
        result.rem -= denom;
    }

    return result;
}

ldiv_t ldiv(long int numer, long int denom) {
    ldiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;

    if (numer >= 0 && result.rem < 0) {
        result.quot++;
        result.rem -= denom;
    }

    return result;
}

int abs(int x) {
    return (int)labs((long int)x);
}

long int labs(long int x) {
    return x < 0 ? -x : x;
}

static uint32_t rand_next = 1;

int rand(void) {
    rand_next = rand_next * 1103515245 + 12345;
    return (rand_next / 65536) % 32768;
}

void srand(unsigned int seed) {
    rand_next = seed;
}

int mblen(const char *pmb, size_t max) {
    return 0;
}

size_t mbstowcs(wchar_t *dest, const char *src, size_t max) {
    return 0;
}

int mbtowc(wchar_t *pwc, const char *pmb, size_t max) {
    return 0;
}

size_t wcstombs(char *dest, const wchar_t *src, size_t max) {
    return 0;
}

int wctomb(char *pmb, wchar_t wc) {
    return 0;
}
