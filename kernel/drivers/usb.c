#include "usb.h"

#include "../bits.h"
#include "../debug.h"

void usb_init(pci_device *device, pci_header *header, uint8_t bus, uint8_t slot, uint8_t func) {
    dbgprint("Initializing USB controller\n");

    switch (header->prog_if) {
        case 0x00:
            // USB 1.0 (Intel)
            dbgprint("\tUHCI Controller\n");
            break;

        case 0x10:
            // USB 1.0 (Microsoft)
            dbgprint("\tOHCI Controller\n");
            break;

        case 0x20:
            // USB 2.0
            dbgprint("\tEHCI Controller\n");
            break;

        case 0x30:
            // USB 3.0
            dbgprint("\tXHCI Controller\n");
            break;
    }
}
