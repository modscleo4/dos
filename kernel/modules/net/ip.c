#include "ip.h"

#define DEBUG 0

#include <stdlib.h>
#include <string.h>
#include "arp.h"
#include "icmp.h"
#include "tcp.h"
#include "udp.h"
#include "../../bits.h"
#include "../../debug.h"

bool ipv4_is_cidr_subnet(uint8_t ip[4], uint8_t netip[4], uint8_t mask[4]) {
    return (
        (ip[0] & mask[0]) == (netip[0] & mask[0])
        && (ip[1] & mask[1]) == (netip[1] & mask[1])
        && (ip[2] & mask[2]) == (netip[2] & mask[2])
        && (ip[3] & mask[3]) == (netip[3] & mask[3])
    );
}

static uint16_t ipv4_calculate_checksum(ipv4_packet *packet) {
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

bool ipv4_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint8_t destination_ip[4], uint8_t protocol, void *protocol_packet, size_t protocol_size, void *data, size_t data_size) {
    dbgprint("ip_send_packet\n");

    uint8_t destination_mac[6];
    if (destination_ip[0] == 255 && destination_ip[1] == 255 && destination_ip[2] == 255 && destination_ip[3] == 255) {
        // Broadcast
        memset(destination_mac, 0xFF, 6);
    } else if (ipv4_is_cidr_subnet(destination_ip, driver->ipv4.ip, driver->ipv4.netmask) && !arp_get_mac(driver, destination_ip, destination_mac, 100)) {
        // IP is in subnet, but no MAC address is known
        dbgprint("Failed to get MAC address for destination IP address %d.%d.%d.%d\n", destination_ip[0], destination_ip[1], destination_ip[2], destination_ip[3]);
        return false;
    } else {
        // Send to gateway
        if (!arp_get_mac(driver, driver->ipv4.gateway, destination_mac, 100)) {
            dbgprint("Failed to get MAC address for gateway IP address %d.%d.%d.%d\n", driver->ipv4.gateway[0], driver->ipv4.gateway[1], driver->ipv4.gateway[2], driver->ipv4.gateway[3]);
            return false;
        }
    }

    ipv4_packet *packet = malloc(sizeof(ipv4_packet) + protocol_size + data_size);
    packet->version = 4;
    packet->ihl = 5;
    packet->dscp = 0;
    packet->ecn = 0;
    packet->total_length = switch_endian_16(sizeof(ipv4_packet) + protocol_size + data_size);
    packet->identification = switch_endian_16(0);
    packet->flags = 0;
    packet->fragment_offset = switch_endian_16(0);
    packet->ttl = 64;
    packet->protocol = protocol;
    packet->header_checksum = switch_endian_16(0);
    memcpy(&packet->source_ip, source_ip, 4);
    memcpy(&packet->destination_ip, destination_ip, 4);

    memcpy((uint8_t *)packet + sizeof(ipv4_packet), protocol_packet, protocol_size);
    memcpy((uint8_t *)packet + sizeof(ipv4_packet) + protocol_size, data, data_size);

    packet->header_checksum = ipv4_calculate_checksum(packet);

    ethernet_send_packet(driver, destination_mac, ETHERTYPE_IPV4, packet, sizeof(ipv4_packet) + protocol_size + data_size);

    free(packet);

    return true;
}

void ipv4_receive_packet(ethernet_driver *driver, ipv4_packet *packet, void *data, size_t data_size) {
    dbgprint("ip_receive_packet\n");

    if (packet->version != 4) {
        dbgprint("Invalid IP version %d\n", packet->version);
        return;
    }

    uint16_t checksum = packet->header_checksum;
    packet->header_checksum = 0;
    if (checksum != ipv4_calculate_checksum(packet)) {
        dbgprint("Invalid IP checksum\n");
        return;
    }
    packet->header_checksum = checksum;

    switch (packet->protocol) {
        case IP_PROTOCOL_ICMP:
            icmp_receive_packet(driver, packet, data, data + sizeof(icmp_packet));
            break;
        case IP_PROTOCOL_UDP:
            udp_receive_packet(driver, packet, data, data + sizeof(udp_packet));
            break;
        case IP_PROTOCOL_TCP:
            tcp_receive_packet(driver, packet, data, data + sizeof(tcp_packet));
            break;
    }
}
