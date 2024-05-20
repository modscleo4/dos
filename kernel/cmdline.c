#include "cmdline.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "cpu/panic.h"

static void validate_cmdline_root(const char *cmdline) {
    // format: root=fdxpy, root=hdxpy, root=cdxpy
    if (strstr(cmdline, "root=")) { // check if root device is specified
        char *tmp = strstr(cmdline, "root=") + 5;
        if (strncmp(tmp, "fd", 2) == 0) { // floppy disk
            tmp += 2;
        } else if (strncmp(tmp, "hd", 2) == 0) { // hard disk
            tmp += 2;
        } else if (strncmp(tmp, "cd", 2) == 0) { // CD-ROM
            tmp += 2;
        } else {
            goto error;
        }

        char *start_disk = tmp;
        if (!isdigit(*start_disk)) { // check if the disk number start with a digit
            goto error;
        }

        char *end_disk = start_disk;
        while (*end_disk && isdigit(*end_disk)) {
            end_disk++;
        }

        if (*end_disk != 'p') { // check if there is a partition number
            goto error;
        }

        char *start_partition = end_disk + 1;
        if (!isdigit(*start_partition)) { // check if the partition number start with a digit
            goto error;
        }

        char *end_partition = start_partition;
        while (*end_partition && isdigit(*end_partition)) {
            end_partition++;
        }

        // check if there is anything after the partition number that is not a whitespace
        if (*end_partition != 0 && !isspace(*end_partition)) {
            goto error;
        }

        return;
    }

error:
    panic("Invalid root device specified");
}

void parse_cmdline_root(const char *cmdline, int *drive, int *partition) {
    validate_cmdline_root(cmdline);

    char *tmp = strstr(cmdline, "root=");
    if (tmp) {
        tmp += 5;
        // format: root=fdxpy, root=hdxpy, root=cdxpy
        if (strncmp(tmp, "fd", 2) == 0) {
            *drive = 0;
            tmp += 2;
        } else if (strncmp(tmp, "hd", 2) == 0) {
            *drive = 0x80;
            tmp += 2;
        } else if (strncmp(tmp, "cd", 2) == 0) {
            *drive = 0xE0;
            tmp += 2;
        }

        char *end = tmp;
        while (*end && *end != 'p') {
            end++;
        }

        char tmp2 = tmp[end - tmp];
        tmp[end - tmp] = 0;
        *drive += strtol(tmp, NULL, 10);
        tmp[end - tmp] = tmp2;

        tmp = end + 1;

        end = tmp;
        while (*end && !isspace(*end)) {
            end++;
        }

        tmp2 = tmp[end - tmp];
        tmp[end - tmp] = 0;
        *partition = strtol(tmp, NULL, 10);
        tmp[end - tmp] = tmp2;
    }
}

static void validate_cmdline_memtest(const char *cmdline) {
    // format: memtest=0, memtest=1
    if (strstr(cmdline, "memtest=")) { // check if memory test is specified
        char *tmp = strstr(cmdline, "memtest=") + 8;
        if (strlen(tmp) != 1 && *tmp != '0' && *tmp != '1') {
            goto error;
        }

        return;
    }

error:
    panic("Invalid memory test specified");
}

bool parse_cmdline_memtest(const char *cmdline) {
    validate_cmdline_memtest(cmdline);

    char *tmp = strstr(cmdline, "memtest=");
    if (tmp) {
        tmp += 8;
        return *tmp == '1';
    }
}

static void validate_cmdline_kblayout(const char *cmdline) {
    // format: kblayout=us
    if (strstr(cmdline, "kblayout=")) { // check if keyboard layout is specified
        char *tmp = strstr(cmdline, "kblayout=") + 9;
        size_t len = strlen(tmp);
        if (len < 2) {
            goto error;
        }

        return;
    }

error:
    panic("Invalid keyboard layout specified");
}

void parse_cmdline_kblayout(const char *cmdline, char *layout) {
    validate_cmdline_kblayout(cmdline);

    char *tmp = strstr(cmdline, "kblayout=");
    if (tmp) {
        tmp += 9;
        size_t len = strlen(tmp);
        strncpy(layout, tmp, len);
    }
}
