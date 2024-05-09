#include "tcp.h"

#define DEBUG 0

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../timer.h"
#include "../../bits.h"
#include "../../debug.h"

static uint32_t syn_c = 0;
static tcp_listener *tcp_listeners = NULL;

static const size_t tcp_syn_cache_size = 16;

typedef struct tcp_syn_cache_entry {
    uint16_t port;
    bool syn;
    uint32_t ack;
} tcp_syn_cache_entry;

static tcp_syn_cache_entry *tcp_syn_cache;
static size_t curr_tcp_syn = 0;

void tcp_init(void) {
    tcp_syn_cache = calloc(tcp_syn_cache_size, sizeof(tcp_syn_cache_entry));
    tcp_listeners = calloc(65536, sizeof(tcp_listener));
}

uint16_t tcp_calculate_checksum(tcp_packet *packet, uint8_t source_ip[4], uint8_t destination_ip[4], void *data, size_t data_size) {
    ipv4_pseudo_header ipv4_header;
    tcp_packet pseudo_header;
    memcpy(ipv4_header.source_ip, source_ip, 4);
    memcpy(ipv4_header.destination_ip, destination_ip, 4);
    ipv4_header.zero = 0;
    ipv4_header.protocol = IP_PROTOCOL_TCP;
    ipv4_header.length = htons(data_size + sizeof(tcp_packet));
    memcpy(&pseudo_header, packet, sizeof(tcp_packet));
    pseudo_header.checksum = 0;

    uint32_t checksum = 0;

    uint16_t *data16 = (uint16_t *)&ipv4_header;
    for (size_t i = 0; i < sizeof(ipv4_pseudo_header) / 2; i++) {
        checksum += data16[i];
    }

    data16 = (uint16_t *)&pseudo_header;
    for (size_t i = 0; i < sizeof(tcp_packet) / 2; i++) {
        checksum += data16[i];
    }

    data16 = (uint16_t *)data;
    for (size_t i = 0; i < data_size / 2; i++) {
        checksum += data16[i];
    }

    if (data_size % 2) {
        checksum += ((uint8_t *)data)[data_size - 1];
    }

    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }

    return ~checksum;
}

void tcp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, uint32_t seq, uint32_t ack, bool is_syn, bool is_ack, bool is_fin, bool is_data, void *data, size_t data_size) {
    dbgprint("tcp_send_packet\n");
    tcp_packet packet;
    memset(&packet, 0, sizeof(tcp_packet));
    packet.source_port = htons(source_port);
    packet.destination_port = htons(destination_port);
    packet.sequence_number = htonl(seq == 0 ? syn_c++ : seq);
    packet.acknowledgement_number = htonl(ack);
    packet.data_offset = sizeof(tcp_packet) / sizeof(uint32_t);
    packet.reserved = 0;
    packet.flags.syn = is_syn;
    packet.flags.ack = is_ack;
    packet.flags.fin = is_fin;
    packet.flags.psh = is_data;
    packet.window_size = htons(32768);
    packet.checksum = tcp_calculate_checksum(&packet, source_ip, destination_ip, data, data_size);
    packet.urgent_pointer = htons(0);

    ipv4_send_packet(driver, source_ip, destination_ip, IP_PROTOCOL_TCP, &packet, sizeof(tcp_packet), data, data_size);
}

bool tcp_syn(ethernet_driver *driver, uint8_t destination_ip[4], uint16_t destination_port, uint16_t source_port, int timeout, uint32_t *ack) {
    tcp_syn_cache[curr_tcp_syn].port = source_port;
    tcp_syn_cache[curr_tcp_syn].syn = false;
    tcp_syn_cache[curr_tcp_syn].ack = 0;
    curr_tcp_syn++;

    tcp_send_packet(driver, driver->ipv4.ip, source_port, destination_ip, destination_port, 0, 0, true, false, false, false, NULL, 0);

    for (int t = 0; t < timeout; t += 10) {
        timer_wait(10);

        for (int i = 0; i < curr_tcp_syn; i++) {
            if (tcp_syn_cache[i].port == source_port) {
                if (tcp_syn_cache[i].syn) {
                    dbgprint("TCP connection established in %d ms\n", t + 10);
                    *ack = tcp_syn_cache[i].ack;
                    return true;
                }
            }
        }
    }

    dbgprint("TCP connection timed out\n");

    return false;
}

void tcp_close_connection(ethernet_driver *driver, uint8_t destination_ip[4], uint16_t destination_port, uint16_t source_port) {
    tcp_send_packet(driver, driver->ipv4.ip, source_port, destination_ip, destination_port, 0, 0, false, true, true, false, NULL, 0);
}

bool tcp_install_listener(uint16_t port, tcp_listener listener) {
    if (tcp_listeners[port]) {
        return false;
    }

    tcp_listeners[port] = listener;
    return true;
}

void tcp_uninstall_listener(uint16_t port) {
    tcp_listeners[port] = NULL;
}

void tcp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_pkt, tcp_packet *packet, void *data) {
    if (!tcp_listeners) {
        return;
    }

    uint16_t port = ntohs(packet->destination_port);
    size_t len = ntohs(ipv4_pkt->total_length) - sizeof(ipv4_packet) - sizeof(tcp_packet);
    size_t data_len = ntohs(ipv4_pkt->total_length) - sizeof(ipv4_packet) - packet->data_offset * sizeof(uint32_t);
    uint16_t checksum = packet->checksum;
    dbgprint("tcp_receive_packet %d (%x)\n", port, len);

    packet->checksum = 0;
    if (checksum != tcp_calculate_checksum(packet, ipv4_pkt->source_ip, ipv4_pkt->destination_ip, data, len)) {
        dbgprint("tcp_receive_packet: checksum failed (got %x)\n", checksum);
        return;
    }
    packet->checksum = checksum;

    if (packet->flags.syn && packet->flags.ack) { // Received SYN-ACK, send ACK
        tcp_send_packet(driver, driver->ipv4.ip, port, ipv4_pkt->source_ip, ntohs(packet->source_port), ntohl(packet->acknowledgement_number), ntohl(packet->sequence_number) + 1, false, true, false, false, NULL, 0);
        for (int i = 0; i < curr_tcp_syn; i++) {
            if (tcp_syn_cache[i].port == port) {
                tcp_syn_cache[i].syn = true;
                tcp_syn_cache[i].ack = ntohl(packet->acknowledgement_number);
                break;
            }
        }

        return;
    }

    if (tcp_listeners[port]) {
        tcp_listener listener = tcp_listeners[port];
        if (listener(driver, data + ((packet->data_offset - 5) * sizeof(uint32_t)), data_len)) {
            // Send ACK
            tcp_send_packet(driver, driver->ipv4.ip, port, ipv4_pkt->source_ip, ntohs(packet->source_port), ntohl(packet->acknowledgement_number), ntohl(packet->sequence_number) + 1, false, true, false, false, NULL, 0);
        }
    }

    if (packet->flags.fin) {
        dbgprint("tcp_receive_packet: FIN\n");
        for (int i = 0; i < curr_tcp_syn; i++) {
            if (tcp_syn_cache[i].port == port) {
                tcp_syn_cache[i].syn = false;
                tcp_syn_cache[i].ack = 0;
                break;
            }
        }
    }
}
