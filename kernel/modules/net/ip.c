#include "ip.h"

#include <stdlib.h>
#include <string.h>
#include "arp.h"
#include "udp.h"
#include "../../bits.h"

uint16_t ipv4_calculate_checksum(ipv4_packet *packet) {
    uint32_t checksum = 0;

    uint16_t *data16 = (uint16_t *)packet;
    for (size_t i = 0; i < sizeof(ipv4_packet) / 2; i++) {
        checksum += data16[i];
    }

    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }

    return ~checksum;
}

void ipv4_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint8_t destination_ip[4], uint8_t protocol, void *protocol_packet, size_t protocol_size, void *data, size_t data_size) {
    dbgprint("ip_send_packet\n");

    ipv4_packet *packet = malloc(sizeof(ipv4_packet) + protocol_size + data_size);
    packet->version = 4;
    packet->ihl = 5;
    packet->dscp = 0;
    packet->ecn = 0;
    packet->total_length = switch_endian_16(sizeof(ipv4_packet) + protocol_size + data_size);
    packet->identification = switch_endian_16(0);
    packet->flags = switch_endian_16(0);
    packet->fragment_offset = switch_endian_16(0);
    packet->ttl = 64;
    packet->protocol = protocol;
    packet->header_checksum = switch_endian_16(0);
    memcpy(&packet->source_ip, source_ip, 4);
    memcpy(&packet->destination_ip, destination_ip, 4);

    memcpy((uint8_t *)packet + sizeof(ipv4_packet), protocol_packet, protocol_size);
    memcpy((uint8_t *)packet + sizeof(ipv4_packet) + protocol_size, data, data_size);

    packet->header_checksum = ipv4_calculate_checksum(packet);

    ethernet_send_packet(driver, (uint8_t[]){255, 255, 255, 255, 255, 255}, ETHERTYPE_IPV4, packet, sizeof(ipv4_packet) + protocol_size + data_size);

    free(packet);
}

void ipv4_receive_packet(ethernet_driver *driver, ipv4_packet *packet, void *data, size_t data_size) {
    dbgprint("ip_receive_packet\n");

    switch (packet->protocol) {
        case IP_PROTOCOL_ICMP:
            //icmp_receive_packet(driver, packet, data, data_size);
            break;
        case IP_PROTOCOL_UDP:
            udp_receive_packet(driver, data, data + sizeof(udp_packet));
            break;
        case IP_PROTOCOL_TCP:
            //tcp_receive_packet(driver, packet, data, data_size);
            break;
    }
}
