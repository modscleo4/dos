#include "ethernet.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "eth/e1000.h"
#include "eth/ne2k.h"
#include "../bits.h"
#include "../debug.h"
#include "../cpu/irq.h"
#include "../modules/net/arp.h"
#include "../modules/net/dhcp.h"
#include "../modules/net/ip.h"

ethernet_driver *eth[2];

static void ethernet_handler(registers *r, uint32_t int_no) {
    dbgprint("ethernet_handler: %d\n", int_no);
    for (int i = 0; i < 2; i++) {
        if (!eth[i]) {
            continue;
        }

        if (eth[i]->int_no == int_no && eth[i]->int_handler) {
            eth[i]->int_handler(eth[i]);
        }
    }
}

static bool ethernet_assign_driver(ethernet_driver *driver) {
    for (int i = 0; i < 2; i++) {
        if (!eth[i]) {
            eth[i] = driver;
            dbgprint("Assigned ethernet driver to interface %d\n", i);
            return true;
        }
    }

    return false;
}

void ethernet_init(pci_device *device, pci_header *header, uint8_t bus, uint8_t slot, uint8_t func) {
    ethernet_driver *driver = NULL;
    switch (header->vendor) {
        case 0x8086: // Intel
            switch (header->device) {
                case 0x100E: // e1000
                    driver = e1000_init(device, bus, slot, func);
                    break;
            }
            break;

        case 0x10EC: // Realtek
            switch (header->device) {
                case 0x8029: // ne2k
                    driver = ne2k_init(device, bus, slot, func);
                    break;
            }
    }

    if (!driver) {
        dbgprint("Unknown ethernet device!\n");
        return;
    }

    if (!driver->mac[0] && !driver->mac[1] && !driver->mac[2] && !driver->mac[3] && !driver->mac[4] && !driver->mac[5]) {
        return;
    }

    driver->int_no = device->interrupt_line;

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

void ethernet_send_packet(ethernet_driver *driver, uint8_t destination_mac[6], enum EtherType ethertype, void *data, size_t data_size) {
    ethernet_packet eth_packet;
    memcpy(eth_packet.header.source_mac, driver->mac, 6);
    memcpy(eth_packet.header.destination_mac, destination_mac, 6);
    eth_packet.header.ethertype = htons(ethertype);
    eth_packet.data = data;

    driver->write(driver, &eth_packet, data_size);
}

void ethernet_process_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    dbgprint("ethernet_process_packet: %dB, %x\n", data_size, ntohs(packet->header.ethertype));
    switch (ntohs(packet->header.ethertype)) {
        case ETHERTYPE_ARP:
            arp_receive_packet(driver, (arp_packet *) packet->data);
            break;

        case ETHERTYPE_IPV4:
            ipv4_receive_packet(driver, (ipv4_packet *) packet->data, packet->data + sizeof(ipv4_packet), data_size - sizeof(ethernet_header));
            break;
    }
}
