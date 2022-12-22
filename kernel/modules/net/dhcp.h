#ifndef DHCP_H
#define DHCP_H

#include <stdint.h>
#include "../../drivers/ethernet.h"

typedef struct dhcp_packet {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic_cookie;
    uint8_t options[312];
} dhcp_packet;

enum DHCPOperation {
    DHCP_OP_DISCOVER = 0x01,
    DHCP_OP_OFFER = 0x02,
    DHCP_OP_REQUEST = 0x03,
    DHCP_OP_DECLINE = 0x04,
    DHCP_OP_ACK = 0x05,
    DHCP_OP_NAK = 0x06,
    DHCP_OP_RELEASE = 0x07,
    DHCP_OP_INFORM = 0x08,
};

enum DHCPHardwareType {
    DHCP_HTYPE_ETHERNET = 0x01,
};

enum DHCPOption {
    DHCP_OPTION_SUBNET_MASK = 0x01,
    DHCP_OPTION_ROUTER = 0x03,
    DHCP_OPTION_DNS_SERVER = 0x06,
    DHCP_OPTION_HOST_NAME = 0x0c,
    DHCP_OPTION_DOMAIN_NAME = 0x0f,
    DHCP_OPTION_REQUESTED_IP = 0x32,
    DHCP_OPTION_LEASE_TIME = 0x33,
    DHCP_OPTION_MESSAGE_TYPE = 0x35,
    DHCP_OPTION_SERVER_IDENTIFIER = 0x36,
    DHCP_OPTION_PARAMETER_REQUEST_LIST = 0x37,
    DHCP_OPTION_END = 0xff,
};

void dhcp_init(ethernet_driver *driver);

void dhcp_send_discover(ethernet_driver *driver);

void dhcp_send_request(ethernet_driver *driver, uint8_t server_ip[4], uint8_t requested_ip[4]);

#endif // DHCP_H
