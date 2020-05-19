#include "floppy.h"

unsigned int boot_drive = 0;
bios_params params;

void initfloppy() {
    unsigned int edx = 0;
    asm("mov %%edx, %0;"
    : "=r" (edx));
    boot_drive = edx >> 16;

    //floppy_read(boot_drive, 0, 0, 1, 1, sizeof(params), &params);
}

void loadfat() {
    puts("Reading from drive ");
    char buff[10];
    itoa(boot_drive, buff, 10);
    puts(buff);
    puts("\n");
}

chs lba2chs(long int lba) {
    return (chs) {
        .cylinder = lba / (2 * params.sectors_per_track),
        .head = ((lba % (2 * params.sectors_per_track)) / params.sectors_per_track),
        .sector = ((lba % (2 * params.sectors_per_track)) % params.sectors_per_track + 1)
    };
}

int floppy_read(int drive, int head, int track, int sector, int sectors, int bytes, char *data) {
    return 0;
}

int floppy_write(int drive, int head, int track, int sector, int sectors, int bytes, char *data) {
    return 0;
}
