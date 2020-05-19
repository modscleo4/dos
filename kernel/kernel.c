#include <stdio.h>
#include "drivers/screen.h"
#include "drivers/floppy.h"

int main() {
    initfloppy();
    initvideo();
    puts("Kernel started\n");
    puts("Reading File Allocation Table...\n");
    loadfat();

    for (;;) {

    }

    return 0;
}
