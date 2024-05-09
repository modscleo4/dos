#include "udp.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../../debug.h"
#include "../../bits.h"

static udp_listener *udp_listeners = NULL;

void udp_init(void) {
    udp_listeners = calloc(65536, sizeof(udp_listener));
}

uint16_t udp_calculate_checksum(udp_packet *packet, uint8_t source_ip[4], uint8_t destination_ip[4], void *data, size_t data_size) {
    ipv4_pseudo_header ipv4_header;
    udp_packet pseudo_header;
    memcpy(ipv4_header.source_ip, source_ip, 4);
    memcpy(ipv4_header.destination_ip, destination_ip, 4);
    ipv4_header.zero = 0;
    ipv4_header.protocol = IP_PROTOCOL_UDP;
    ipv4_header.length = packet->length;
    memcpy(&pseudo_header, packet, sizeof(udp_packet));
    pseudo_header.checksum = 0;

    uint32_t checksum = 0;

    uint16_t *data16 = (uint16_t *)&ipv4_header;
    for (size_t i = 0; i < sizeof(ipv4_pseudo_header) / 2; i++) {
        checksum += data16[i];
    }

    data16 = (uint16_t *)&pseudo_header;
    for (size_t i = 0; i < sizeof(udp_packet) / 2; i++) {
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

bool udp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size) {
    dbgprint("udp_send_packet\n");
    udp_packet packet;
    memset(&packet, 0, sizeof(udp_packet));
    packet.source_port = htons(source_port);
    packet.destination_port = htons(destination_port);
    packet.length = htons(data_size + sizeof(udp_packet));
    packet.checksum = udp_calculate_checksum(&packet, source_ip, destination_ip, data, data_size);

    return ipv4_send_packet(driver, source_ip, destination_ip, IP_PROTOCOL_UDP, &packet, sizeof(udp_packet), data, data_size);
}

bool udp_install_listener(uint16_t port, udp_listener listener) {
    if (udp_listeners[port]) {
        return false;
    }

    udp_listeners[port] = listener;
    return true;
}

void udp_uninstall_listener(uint16_t port) {
    udp_listeners[port] = NULL;
}

void udp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, udp_packet *packet, void *data) {
    if (!udp_listeners) {
        return;
    }

    dbgprint("udp_receive_packet\n");
    uint16_t port = ntohs(packet->destination_port);
    uint16_t checksum = packet->checksum;

    packet->checksum = 0;
    if (checksum != udp_calculate_checksum(packet, ipv4_packet->source_ip, ipv4_packet->destination_ip, data, ntohs(packet->length) - sizeof(udp_packet))) {
        dbgprint("udp_receive_packet: checksum failed\n");
        return;
    }
    packet->checksum = checksum;

    if (udp_listeners[port]) {
        udp_listener listener = udp_listeners[port];
        listener(driver, data, ntohs(packet->length) - sizeof(udp_packet));
    }
}
