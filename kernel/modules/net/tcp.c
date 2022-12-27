#include "tcp.h"

#include <stdlib.h>
#include "../../debug.h"
#include "../../bits.h"

static tcp_listener *tcp_listeners = NULL;

void tcp_init(void) {
    tcp_listeners = calloc(65536, sizeof(tcp_listener));
}

void tcp_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint16_t source_port, uint8_t destination_ip[4], uint16_t destination_port, void *data, size_t data_size) {
    dbgprint("tcp_send_packet\n");
    static tcp_packet packet;
    packet.source_port = switch_endian_16(source_port);
    packet.destination_port = switch_endian_16(destination_port);
    packet.sequence_number = switch_endian_32(0);
    packet.acknowledgement_number = switch_endian_32(0);
    packet.data_offset = 5;
    packet.reserved = 0;
    //packet.flags = 0;
    packet.window_size = switch_endian_16(0);
    packet.checksum = switch_endian_16(0);
    packet.urgent_pointer = switch_endian_16(0);

    ipv4_send_packet(driver, source_ip, destination_ip, IP_PROTOCOL_TCP, &packet, sizeof(tcp_packet), data, data_size);
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

void tcp_receive_packet(ethernet_driver *driver, ipv4_packet *ipv4_packet, tcp_packet *packet, void *data) {

}
