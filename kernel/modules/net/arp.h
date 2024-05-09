#ifndef ARP_H
#define ARP_H

#include <stdbool.h>
#include <stdint.h>
#include "../../drivers/ethernet.h"

#pragma pack(push, 1)
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
#pragma pack(pop)

enum ARPHardwareType {
    ARP_HW_ETHERNET = 0x0001,
};

enum ARPProtocolType {
    ARP_PROTO_IPV4 = 0x0800,
};

enum ARPOperation {
    ARP_OP_REQUEST = 0x01,
    ARP_OP_REPLY = 0x02,
};

bool arp_get_mac(ethernet_driver *driver, uint8_t ip[4], uint8_t out_mac[6], int timeout);

void arp_send_request(ethernet_driver *driver, uint8_t ip[4]);

void arp_send_reply(ethernet_driver *driver, uint8_t ip[4], uint8_t mac[6]);

void arp_receive_packet(ethernet_driver *driver, arp_packet *packet);

#endif // ARP_H
