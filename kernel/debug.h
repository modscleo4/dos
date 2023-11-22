#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL 0
#endif

#if DEBUG

#if DEBUG_SERIAL
#define _dbgprint _serial_dbgprint
#else
#define _dbgprint _screen_dbgprint
#endif

#define dbgprint(...) _dbgprint(__FILE__, __LINE__, __VA_ARGS__)
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

void _screen_dbgprint(const char *filename, int line, const char *msg, ...);
void _serial_dbgprint(const char *filename, int line, const char *msg, ...);

void _dbgwait(void);

void hexdump(void *ptr, size_t n);

void callstack(uint32_t ebp);

#endif // DEBUG_H
