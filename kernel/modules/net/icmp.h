#ifndef ICMP_H
#define ICMP_H

#include <stdbool.h>
#include <stddef.h>
#include "ip.h"

#pragma pack(push, 1)
typedef struct icmp_packet {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t identifier;
            uint16_t sequence_number;
        } echo;

        struct {
            uint16_t unused;
            uint16_t mtu;
        } fragmentation;
    };
    uint8_t data[];
} icmp_packet;
#pragma pack(pop)

void icmp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, icmp_packet *packet, void *data);

#endif // ICMP_H
