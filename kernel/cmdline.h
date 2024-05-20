#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdbool.h>

void parse_cmdline_root(const char *cmdline, int *drive, int *partition);

bool parse_cmdline_memtest(const char *cmdline);

void parse_cmdline_kblayout(const char *cmdline, char *kblayout);

#endif // CMDLINE_H
