#include "power.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include "acpi.h"
#include "interrupts.h"
#include "../bits.h"
#include "../debug.h"
#include "../rootfs.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

static void before_shutdown(void) {
    keyboard_clear_buffer();

    screen_setcolor(COLOR_BLACK << 4 | COLOR_GRAY);
    screen_clear();

    if (rootfs_io.device != -2) {
        rootfs_io.stop(&rootfs_io);
    }
}

void power_shutdown(void) {
    before_shutdown();

    acpi_shutdown();
    dbgprint("ACPI shutdown failed!\n");

    interrupts_disable();

    outw(0x604, 0x2000); // QEMU
    outw(0xB004, 0x0 | 0x2000); // Bochs
    outw(0x4004, 0x3400); // VirtualBox
    outw(0x600, 0x34); // VMware

    while (true) {
        asm volatile("hlt");
    }
}

void power_reboot(void) {
    before_shutdown();

    acpi_reboot();
    dbgprint("ACPI reboot failed!\n");

    interrupts_disable();

    keyboard_clear_buffer();
    outb(KB_CTRL_REGISTER, KB_RESET);

    while (true) {
        asm volatile("hlt");
    }
}
