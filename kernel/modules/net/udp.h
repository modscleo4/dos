#ifndef TCP_H
#define TCP_H

#include "ip.h"
#include "../../drivers/ethernet.h"

typedef struct udp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
    void *data;
} udp_packet;

void udp_send_packet(ethernet_driver *driver, uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size);

#endif // TCP_H
