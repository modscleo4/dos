#ifndef ICMP_H
#define ICMP_H

#include <stdbool.h>
#include <stddef.h>
#include "ip.h"

typedef struct icmp_packet {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t identifier;
            uint16_t sequence_number;
        } __attribute__((packed)) echo;
        struct {
            uint16_t unused;
            uint16_t mtu;
        } __attribute__((packed)) fragmentation;
    };
    uint8_t data[];
} __attribute__((packed)) icmp_packet;

void icmp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, icmp_packet *packet, void *data);

#endif // ICMP_H
