#include "ethernet.h"

#include <string.h>
#include "../../cpu/irq.h"
#include "e1000.h"
#include "ne2k.h"
#include "../modules/net/arp.h"
#include "../modules/net/dhcp.h"
#include "../../bits.h"
#include "../../debug.h"

static void ethernet_handler(registers *r) {
    for (int i = 0; i < 2; i++) {
        if (eth[i].mac[0] || eth[i].mac[1] || eth[i].mac[2] || eth[i].mac[3] || eth[i].mac[4] || eth[i].mac[5]) {
            eth[i].int_handler(&eth[i]);
        }
    }
}

bool ethernet_assign_driver(ethernet_driver *driver) {
    for (int i = 0; i < 2; i++) {
        if (!eth[i].mac[0] && !eth[i].mac[1] && !eth[i].mac[2] && !eth[i].mac[3] && !eth[i].mac[4] && !eth[i].mac[5]) {
            eth[i] = *driver;
            return true;
        }
    }

    return false;
}

void ethernet_init(pci_device *device, pci_header *header) {
    ethernet_driver *driver;
    switch (header->vendor) {
        case 0x8086: // Intel
            switch (header->device) {
                case 0x100E: // e1000
                    driver = e1000_init(device);
                    break;
            }
            break;

        case 0x10EC: // Realtek
            switch (header->device) {
                case 0x8029: // ne2k
                    driver = ne2k_init(device);
                    break;
            }
    }

    if (driver == NULL) {
        dbgprint("Unknown ethernet device!\n");
        return;
    }

    if (!driver->mac[0] && !driver->mac[1] && !driver->mac[2] && !driver->mac[3] && !driver->mac[4] && !driver->mac[5]) {
        return;
    }

    if (!ethernet_assign_driver(driver)) {
        dbgprint("Too many ethernet devices!\n");
        return;
    }

    dbgprint("Interrupt line: %d\n", device->interrupt_line);
    dbgprint("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", driver->mac[0], driver->mac[1], driver->mac[2], driver->mac[3], driver->mac[4], driver->mac[5]);
    dbgprint("Sending DHCP request...\n");

    irq_install_handler(device->interrupt_line, ethernet_handler);
    if (driver->int_enable) {
        driver->int_enable(driver);
    }

    dhcp_init(driver);
}

void ethernet_send_packet(ethernet_driver *driver, uint8_t destination_mac[6], uint16_t ethertype, void *data, size_t data_size) {
    static ethernet_packet eth_packet;
    memcpy(eth_packet.header.source_mac, &driver->mac, 6);
    memset(eth_packet.header.destination_mac, 0xFF, 6);
    eth_packet.header.ethertype = switch_endian_16(ETHERTYPE_ARP);
    eth_packet.data = data;

    driver->write(driver, &eth_packet, data_size);
}

void ethernet_process_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    dbgprint("ethernet_process_packet: %db\n", data_size);
    switch (switch_endian_16(packet->header.ethertype)) {
        case ETHERTYPE_ARP:
            arp_receive_packet(driver, packet->data);
            break;

        case ETHERTYPE_IPV4:
            break;
    }
}
