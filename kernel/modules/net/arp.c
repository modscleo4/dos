#include "arp.h"

#include <string.h>
#include "../timer.h"
#include "../../bits.h"
#include "../../debug.h"

static arp_map arp_list[10];
static int curr_arp = 0;

void arp_get_mac(ethernet_driver *driver, uint8_t ip[4], uint8_t mac[6]) {
    for (int i = 0; i < curr_arp; i++) {
        if (arp_list[i].ip[0] == ip[0] && arp_list[i].ip[1] == ip[1] && arp_list[i].ip[2] == ip[2] && arp_list[i].ip[3] == ip[3]) {
            memcpy(mac, arp_list[i].mac, 6);
            return;
        }
    }

    arp_send_request(driver, ip);
    timer_wait(100);

    for (int i = 0; i < curr_arp; i++) {
        if (arp_list[i].ip[0] == ip[0] && arp_list[i].ip[1] == ip[1] && arp_list[i].ip[2] == ip[2] && arp_list[i].ip[3] == ip[3]) {
            memcpy(mac, arp_list[i].mac, 6);
            return;
        }
    }
}

void arp_send_request(ethernet_driver *driver, uint8_t ip[4]) {
    static arp_packet packet;
    packet.hardware_type = switch_endian_16(0x0001);
    packet.protocol_type = switch_endian_16(0x0800);
    packet.hardware_size = 0x06;
    packet.protocol_size = 0x04;
    packet.opcode = switch_endian_16(ARP_OP_REQUEST);
    for (int i = 0; i < 6; i++) {
        packet.sender_mac[i] = driver->mac[i];
        packet.target_mac[i] = 0x00;
    }

    packet.sender_ip[0] = driver->ipv4.ip[0];
    packet.sender_ip[1] = driver->ipv4.ip[1];
    packet.sender_ip[2] = driver->ipv4.ip[2];
    packet.sender_ip[3] = driver->ipv4.ip[3];

    packet.target_ip[0] = ip[0];
    packet.target_ip[1] = ip[1];
    packet.target_ip[2] = ip[2];
    packet.target_ip[3] = ip[3];

    uint8_t destination_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    dbgprint("ARP request sent to %d.%d.%d.%d\n", packet.target_ip[0], packet.target_ip[1], packet.target_ip[2], packet.target_ip[3]);

    ethernet_send_packet(driver, destination_mac, ETHERTYPE_ARP, &packet, sizeof(arp_packet));
}

void arp_send_reply(ethernet_driver *driver, uint8_t ip[4], uint8_t mac[6]) {
    static arp_packet packet;
    packet.hardware_type = switch_endian_16(0x0001);
    packet.protocol_type = switch_endian_16(0x0800);
    packet.hardware_size = 0x06;
    packet.protocol_size = 0x04;
    packet.opcode = switch_endian_16(ARP_OP_REPLY);
    for (int i = 0; i < 6; i++) {
        packet.sender_mac[i] = driver->mac[i];
        packet.target_mac[i] = mac[i];
    }

    packet.sender_ip[0] = driver->ipv4.ip[0];
    packet.sender_ip[1] = driver->ipv4.ip[1];
    packet.sender_ip[2] = driver->ipv4.ip[2];
    packet.sender_ip[3] = driver->ipv4.ip[3];

    packet.target_ip[0] = ip[0];
    packet.target_ip[1] = ip[1];
    packet.target_ip[2] = ip[2];
    packet.target_ip[3] = ip[3];

    dbgprint("ARP reply sent to %d.%d.%d.%d\n", packet.target_ip[0], packet.target_ip[1], packet.target_ip[2], packet.target_ip[3]);

    ethernet_send_packet(driver, mac, ETHERTYPE_ARP, &packet, sizeof(arp_packet));
}

static void arp_process_request(ethernet_driver *driver, arp_packet *packet) {
    dbgprint("ARP request received from %d.%d.%d.%d\n", packet->sender_ip[0], packet->sender_ip[1], packet->sender_ip[2], packet->sender_ip[3]);
    dbgprint("MAC: %x:%x:%x:%x:%x:%x\n", packet->sender_mac[0], packet->sender_mac[1], packet->sender_mac[2], packet->sender_mac[3], packet->sender_mac[4], packet->sender_mac[5]);

    if (
        driver->ipv4.ip[0] == packet->target_ip[0] &&
        driver->ipv4.ip[1] == packet->target_ip[1] &&
        driver->ipv4.ip[2] == packet->target_ip[2] &&
        driver->ipv4.ip[3] == packet->target_ip[3]
    ) {
        arp_send_reply(driver, packet->sender_ip, packet->sender_mac);
    }
}

static void arp_process_reply(ethernet_driver *driver, arp_packet *packet) {
    dbgprint("ARP reply received from %d.%d.%d.%d\n", packet->sender_ip[0], packet->sender_ip[1], packet->sender_ip[2], packet->sender_ip[3]);
    dbgprint("MAC: %x:%x:%x:%x:%x:%x\n", packet->sender_mac[0], packet->sender_mac[1], packet->sender_mac[2], packet->sender_mac[3], packet->sender_mac[4], packet->sender_mac[5]);

    if (curr_arp < sizeof(arp_list)) {
        memcpy(arp_list[curr_arp].ip, packet->sender_ip, sizeof(arp_list[curr_arp].ip));
        memcpy(arp_list[curr_arp].mac, packet->sender_mac, sizeof(arp_list[curr_arp].mac));
        curr_arp++;
    }
}

void arp_receive_packet(ethernet_driver *driver, arp_packet *packet) {
    dbgprint("ARP packet received\n");
    switch (switch_endian_16(packet->opcode)) {
        case ARP_OP_REQUEST:
            arp_process_request(driver, packet);
            break;
        case ARP_OP_REPLY:
            arp_process_reply(driver, packet);
            break;
    }
}
