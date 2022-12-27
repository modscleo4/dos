#ifndef STDARG_H
#define STDARG_H

#define va_start(va_list, last_arg) __builtin_va_start(va_list, last_arg)
#define va_end(va_list) __builtin_va_end(va_list)
#define va_arg(va_list, last_arg) __builtin_va_arg(va_list, last_arg)

typedef __builtin_va_list va_list;

#endif //STDARG_H
