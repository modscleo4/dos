#ifndef UDP_H
#define UDP_H

#include <stdbool.h>
#include <stdint.h>
#include "ip.h"
#include "../../drivers/ethernet.h"

typedef struct udp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_packet;

typedef void (*udp_listener)(ethernet_driver *, void *, size_t);

void udp_init(void);

bool udp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size);

bool udp_install_listener(uint16_t port, udp_listener listener);

void udp_uninstall_listener(uint16_t port);

void udp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, udp_packet *packet, void *data);


#endif // UDP_H
