#ifndef SIGNAL_H
#define SIGNAL_H

typedef void (*sighandler_t)(int);

#define SIG_IGN ((sighandler_t)1)  /* Request that signal be ignored. */
#define SIG_DFL ((sighandler_t)2)  /* Request for default signal handling. */
#define SIG_HOLD ((sighandler_t)3) /* Request that signal be held. */
#define SIG_ERR ((sighandler_t)-1) /* Return value in case of error. */

enum Signal {
    SIGHUP = 1,     /* Hang up controlling terminal or process */
    SIGINT,         /* Interrupt from keyboard, Control-C */
    SIGQUIT,        /* Quit from keyboard, Control-\ */
    SIGILL,         /* Illegal instruction */
    SIGTRAP,        /* Breakpoint for debugging */
    SIGABRT,        /* Abnormal termination */
    SIGIOT = 6,     /* Equivalent to SIGABRT */
    SIGBUS,         /* Bus error */
    SIGFPE,         /* Floating-point exception */
    SIGKILL,        /* Forced-process termination */
    SIGUSR1,        /* Available to processes */
    SIGSEGV,        /* Invalid memory reference */
    SIGUSR2,        /* Available to processes */
    SIGPIPE,        /* Write to pipe with no readers */
    SIGALRM,        /* Real-timer clock */
    SIGTERM,        /* Process termination */
    SIGSTKFLT,      /* Coprocessor stack error */
    SIGCHLD,        /* Child process stopped or terminated or got a signal if traced */
    SIGCONT,        /* Resume execution, if stopped */
    SIGSTOP,        /* Stop process execution, Ctrl-Z */
    SIGTSTP,        /* Stop process issued from tty */
    SIGTTIN,        /* Background process requires input */
    SIGTTOU,        /* Background process requires output */
    SIGURG,         /* Urgent condition on socket */
    SIGXCPU,        /* CPU time limit exceeded */
    SIGXFSZ,        /* File size limit exceeded */
    SIGVTALRM,      /* Virtual timer clock */
    SIGPROF,        /* Profile timer clock */
    SIGWINCH,       /* Window resizing */
    SIGIO,          /* I/O now possible */
    SIGPOLL = 29,   /* Equivalent to SIGIO */
    SIGPWR,         /* Power supply failure */
    SIGSYS,         /* Bad system call */
    SIGUNUSED = 31, /* Equivalent to SIGSYS */
};

#define NSIG 32

#endif // SIGNAL_H
