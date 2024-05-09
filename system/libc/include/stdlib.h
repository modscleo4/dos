#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
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

float atof(const char *);

int atoi(const char *);

char *htoa(short int, char *, int);

char *itoa(int, char *, int);

char *ltoa(long int, char *, int);

char *hutoa(unsigned short int, char *, int);

char *utoa(unsigned int, char *, int);

char *lutoa(unsigned long int, char *, int);

char *ftoa(float, char *, int);

char *lftoa(double, char *, int);

long int atol(const char *);

double strtod(const char *, char **);

long int strtol(const char *, char **, int);

unsigned long int strtoul(const char *, char **, int);

void *calloc(size_t, size_t);

void free(void *);

void *malloc(size_t);

void *realloc(void *, size_t);

void abort(void);

int atexit(void (*)(void));

void exit(int);

char *getenv(const char *);

int system(const char *);

void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));

void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

int abs(int);

div_t div(int, int);

ldiv_t ldiv(long int, long int);

long int labs(long int);

int rand(void);

void srand(unsigned int seed);

int mblen(const char *, size_t);

size_t mbstowcs(wchar_t *, const char *, size_t);

int mbtowc(wchar_t *, const char *, size_t);

size_t wcstombs(char *, const wchar_t *, size_t);

int wctomb(char *, wchar_t);

#endif //STDLIB_H
