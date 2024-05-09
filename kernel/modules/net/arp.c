#include "arp.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <string.h>
#include <arpa/inet.h>
#include "../timer.h"
#include "../../bits.h"
#include "../../debug.h"

typedef struct arp_map {
    uint8_t ip[4];
    uint8_t mac[6];
} arp_map;

static arp_map arp_list[10];
static size_t curr_arp = 0;

bool arp_get_mac(ethernet_driver *driver, uint8_t ip[4], uint8_t mac[6], int timeout) {
    for (int i = 0; i < curr_arp; i++) {
        if (arp_list[i].ip[0] == ip[0] && arp_list[i].ip[1] == ip[1] && arp_list[i].ip[2] == ip[2] && arp_list[i].ip[3] == ip[3]) {
            memcpy(mac, arp_list[i].mac, 6);
            return true;
        }
    }

    arp_send_request(driver, ip);

    for (int t = 0; t < timeout; t += 10) {
        timer_wait(10);

        for (int i = 0; i < curr_arp; i++) {
            if (arp_list[i].ip[0] == ip[0] && arp_list[i].ip[1] == ip[1] && arp_list[i].ip[2] == ip[2] && arp_list[i].ip[3] == ip[3]) {
                dbgprint("ARP: found MAC for IP %d.%d.%d.%d in %d ms\n", ip[0], ip[1], ip[2], ip[3], t + 10);
                memcpy(mac, arp_list[i].mac, 6);
                return true;
            }
        }
    }

    dbgprint("ARP: failed to find MAC for IP %d.%d.%d.%d in %d ms\n", ip[0], ip[1], ip[2], ip[3], timeout);

    return false;
}

void arp_send_request(ethernet_driver *driver, uint8_t ip[4]) {
    arp_packet packet;
    packet.hardware_type = htons(ARP_HW_ETHERNET);
    packet.protocol_type = htons(ARP_PROTO_IPV4);
    packet.hardware_size = 0x06;
    packet.protocol_size = 0x04;
    packet.opcode = htons(ARP_OP_REQUEST);
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
    arp_packet packet;
    packet.hardware_type = htons(ARP_HW_ETHERNET);
    packet.protocol_type = htons(ARP_PROTO_IPV4);
    packet.hardware_size = 0x06;
    packet.protocol_size = 0x04;
    packet.opcode = htons(ARP_OP_REPLY);
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
        driver->ipv4.ip[0] == packet->target_ip[0]
        && driver->ipv4.ip[1] == packet->target_ip[1]
        && driver->ipv4.ip[2] == packet->target_ip[2]
        && driver->ipv4.ip[3] == packet->target_ip[3]
    ) {
        arp_send_reply(driver, packet->sender_ip, packet->sender_mac);
    }
}

static void arp_process_reply(ethernet_driver *driver, arp_packet *packet) {
    dbgprint("ARP reply received from %d.%d.%d.%d\n", packet->sender_ip[0], packet->sender_ip[1], packet->sender_ip[2], packet->sender_ip[3]);
    dbgprint("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", packet->sender_mac[0], packet->sender_mac[1], packet->sender_mac[2], packet->sender_mac[3], packet->sender_mac[4], packet->sender_mac[5]);

    if (curr_arp < sizeof(arp_list)) {
        memcpy(arp_list[curr_arp].ip, packet->sender_ip, 4);
        memcpy(arp_list[curr_arp].mac, packet->sender_mac, 6);
        curr_arp++;
    }
}

void arp_receive_packet(ethernet_driver *driver, arp_packet *packet) {
    dbgprint("ARP packet received\n");
    switch (ntohs(packet->opcode)) {
        case ARP_OP_REQUEST:
            arp_process_request(driver, packet);
            break;
        case ARP_OP_REPLY:
            arp_process_reply(driver, packet);
            break;
    }
}
