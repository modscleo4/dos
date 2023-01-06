#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define dbgprint(...) _dbgprint(__VA_ARGS__)
#define dbgwait() _dbgwait()
#else
#define dbgprint(...)
#define dbgwait()
#endif

#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void _dbgprint(const char *msg, ...);

void _dbgwait(void);

void hexdump(void *ptr, size_t n);

void callstack(uint32_t ebp);

#endif // DEBUG_H
