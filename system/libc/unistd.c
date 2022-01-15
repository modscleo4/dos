#include <stdarg.h>
#include <unistd.h>

int syscall(int sysno, ...) {
    va_list args;

    long int arg0, arg1, arg2, arg3, arg4, arg5;

    va_start(args, sysno);
    arg0 = va_arg(args, long int);
    arg1 = va_arg(args, long int);
    arg2 = va_arg(args, long int);
    arg3 = va_arg(args, long int);
    arg4 = va_arg(args, long int);
    arg5 = va_arg(args, long int);
    va_end(args);

    int retval;

    asm volatile("push %%ebp;"
        "mov %1, %%ebp;"
        "int $0x80;"
        "pop %%ebp"
        : "=g"(retval)
        : "g"(arg5), "a"(sysno), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3), "D"(arg4)
        : "memory");

    return retval;
}
