#ifndef STDLIB_H
#define STDLIB_H

typedef unsigned int size_t;

typedef int wchar_t;

enum ExitCodes {
    EXIT_SUCCESS = 0,
    EXIT_FAILURE = 1,
};

#define MB_CUR_MAX 8
#ifndef NULL
#define NULL (void *)0
#endif
#define RAND_MAX 32767

typedef struct div_t {
    int quot;
    int rem;
} div_t;

typedef struct ldiv_t {
    long int quot;
    long int rem;
} ldiv_t;

#define _MIN_MALLOC_SIZE 16

float atof(const char *str);

int atoi(const char *str);

char *htoa(short int value, char *str, int base);

char *itoa(int value, char *str, int base);

char *ltoa(long int value, char *str, int base);

char *hutoa(unsigned short int value, char *str, int base);

char *utoa(unsigned int value, char *str, int base);

char *lutoa(unsigned long int value, char *str, int base);

char *ftoa(float value, char *str, int precision);

char *lftoa(double value, char *str, int precision);

long int atol(const char *str);

double strtod(const char *str, char **endptr);

long int strtol(const char *str, char **endptr, int base);

unsigned long int strtoul(const char *str, char **endptr, int base);

void kernel_malloc_init(void);

void *calloc(size_t num, size_t size);

void *calloc_align(size_t num, size_t size, size_t align);

void free(void *ptr);

void *malloc(size_t size);

void *malloc_align(size_t size, size_t align);

void *realloc(void *ptr, size_t size);

void abort(void);

int atexit(void (*func)(void));

void exit(int status);

char *getenv(const char *name);

int system(const char *command);

void *bsearch(const void *key, const void *base, size_t num, size_t size, int (*compar)(const void *, const void *));

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *));

div_t div(int numer, int denom);

ldiv_t ldiv(long int numer, long int denom);

int abs(int x);

long int labs(long int x);

int rand(void);

void srand(unsigned int seed);

int mblen(const char *pmb, size_t max);

size_t mbstowcs(wchar_t *dest, const char *src, size_t max);

int mbtowc(wchar_t *pwc, const char *pmb, size_t max);

size_t wcstombs(char *dest, const wchar_t *src, size_t max);

int wctomb(char *pmb, wchar_t wc);

#endif //STDLIB_H
