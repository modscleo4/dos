#ifndef TCP_H
#define TCP_H

#include "ip.h"
#include "../../drivers/ethernet.h"

typedef struct udp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
} udp_packet;

typedef struct udp_pseudo_header {
    uint8_t source_ip[4];
    uint8_t destination_ip[4];
    uint8_t zero;
    uint8_t protocol;
    uint16_t length;
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t udp_length;
    uint16_t checksum;
} udp_pseudo_header;

typedef void (*udp_listener)(ethernet_driver *, void *, size_t);

void udp_init(void);

void udp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size);

void udp_install_listener(uint16_t port, udp_listener listener);

void udp_uninstall_listener(uint16_t port);

void udp_receive_packet(ethernet_driver *driver, udp_packet *packet, void *data);


#endif // TCP_H
