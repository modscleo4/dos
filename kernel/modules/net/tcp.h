#ifndef TCP_H
#define TCP_H

#include <stdbool.h>
#include <stdint.h>
#include "ip.h"
#include "../../drivers/ethernet.h"

#pragma pack(push, 1)
typedef struct tcp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;
    bool ns: 1;
    uint8_t reserved: 3;
    uint8_t data_offset: 4;
    struct {
        bool fin: 1;
        bool syn: 1;
        bool rst: 1;
        bool psh: 1;
        bool ack: 1;
        bool urg: 1;
        bool ece: 1;
        bool cwr: 1;
    } flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
} tcp_packet;
#pragma pack(pop)

typedef bool (*tcp_listener)(ethernet_driver *, void *, size_t);

void tcp_init(void);

void tcp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, uint32_t seq, uint32_t ack, bool is_syn, bool is_ack, bool is_fin, bool is_data, void *data, size_t data_size);

bool tcp_syn(ethernet_driver *driver, uint8_t destination_ip[4], uint16_t destination_port, uint16_t source_port, int timeout, uint32_t *ack);

void tcp_close_connection(ethernet_driver *driver, uint8_t destination_ip[4], uint16_t destination_port, uint16_t source_port);

bool tcp_install_listener(uint16_t port, tcp_listener listener);

void tcp_uninstall_listener(uint16_t port);

void tcp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, tcp_packet *packet, void *data);

#endif // TCP_H
