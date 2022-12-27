#ifndef TCP_H
#define TCP_H

#include <stdbool.h>
#include <stdint.h>
#include "ip.h"
#include "../../drivers/ethernet.h"

typedef struct tcp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;
    uint8_t data_offset: 4;
    uint8_t reserved: 3;
    struct {
        bool ns: 1;
        bool cwr: 1;
        bool ece: 1;
        bool urg: 1;
        bool ack: 1;
        bool psh: 1;
        bool rst: 1;
        bool syn: 1;
        bool fin: 1;
    } __attribute__((packed)) flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
} __attribute__((packed)) tcp_packet;

typedef void (*tcp_listener)(ethernet_driver *, void *, size_t);

void tcp_init(void);

void tcp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size);

bool tcp_install_listener(uint16_t port, tcp_listener listener);

void tcp_uninstall_listener(uint16_t port);

void tcp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, tcp_packet *packet, void *data);

#endif // TCP_H
