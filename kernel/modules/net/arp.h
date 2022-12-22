#ifndef ARP_H
#define ARP_H

#include <stdint.h>
#include "../../drivers/ethernet.h"

typedef struct arp_packet {
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t target_mac[6];
    uint8_t target_ip[4];
} arp_packet;

typedef struct arp_map {
    uint8_t ip[4];
    uint8_t mac[4];
} arp_map;

enum ARPOperation {
    ARP_OP_REQUEST = 0x01,
    ARP_OP_REPLY = 0x02,
};

void arp_send_request(ethernet_driver *driver, uint8_t ip[4]);

void arp_send_reply(ethernet_driver *driver, uint8_t ip[4], uint8_t mac[6]);

void arp_receive_packet(ethernet_driver *driver, arp_packet *packet);

#endif // ARP_H
