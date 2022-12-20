#include "dhcp.h"

#include "arp.h"
#include "udp.h"
#include "../../debug.h"

void dhcp_init(ethernet_driver *driver) {
    dbgprint("dhcp_init\n");
    //udp_send_packet(driver, 68, (unsigned char[]){192, 168, 1, 1}, 67, );
    dhcp_send_discover(driver);
}

void dhcp_send_discover(ethernet_driver *driver) {
    dbgprint("dhcp_send_discover\n");
    dhcp_discover_packet packet;
    packet.header.op = DHCP_OP_DISCOVER;
    packet.header.htype = DHCP_HTYPE_ETHERNET;
    packet.header.hlen = 6;
    packet.header.hops = 0;
    packet.xid = 0x12345678;
    packet.secs = 0;
    packet.flags = 0;
    memset(&packet.ciaddr, 0, 4);
    memset(&packet.yiaddr, 0, 4);
    memset(&packet.siaddr, 0, 4);
    memset(&packet.giaddr, 0, 4);
    memcpy(&packet.chaddr, &driver->mac, 6);
    memset(&packet.chaddr[6], 0, 10);
    memset(&packet.sname, 0, 64);
    memset(&packet.file, 0, 128);
    packet.magic_cookie = 0x63825363;
    packet.options[0] = DHCP_OPTION_MESSAGE_TYPE;
    packet.options[1] = 1;
    //packet.options[2] = DHCP_MESSAGE_TYPE_DISCOVER;
    packet.options[3] = DHCP_OPTION_END;

    udp_send_packet(driver, 68, (unsigned char[]){255, 255, 255, 255}, 67, &packet, sizeof(dhcp_discover_packet));
    arp_send_request(driver, (unsigned char[]){10, 0, 2, 2});
}
