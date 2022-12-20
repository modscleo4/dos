#include "udp.h"

#include "ip.h"
#include "../../debug.h"

void udp_send_packet(ethernet_driver *driver, unsigned short int source_port, unsigned char destination_ip[4], unsigned short int destination_port, void *data, size_t data_size) {
    dbgprint("udp_send_packet\n");
    //ip_send_packet(driver, destination_ip, IP_PROTOCOL_UDP, data, data_size);
}
