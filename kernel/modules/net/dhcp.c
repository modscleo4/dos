#include "dhcp.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include "arp.h"
#include "udp.h"
#include "../timer.h"
#include "../../debug.h"
#include "../../bits.h"

typedef struct dhcp_offer {
    uint8_t server_ip[4];
    uint8_t offered_ip[4];
} dhcp_offer;

static dhcp_offer offers[2];
static int offer_count = 0;

static void dhcp_process_offer(ethernet_driver *driver, dhcp_packet *packet) {
    dbgprint("Received DHCP offer from %d.%d.%d.%d: %d.%d.%d.%d\n", packet->siaddr[0], packet->siaddr[1], packet->siaddr[2], packet->siaddr[3], packet->yiaddr[0], packet->yiaddr[1], packet->yiaddr[2], packet->yiaddr[3]);
    //arp_send_request(driver, packet->yiaddr);

    if (offer_count >= 2) {
        dbgprint("Received too many DHCP offers\n");
        return;
    }

    memcpy(offers[offer_count].server_ip, packet->siaddr, 4);
    memcpy(offers[offer_count].offered_ip, packet->yiaddr, 4);
    offer_count++;
}

static void dhcp_process_ack(ethernet_driver *driver, dhcp_packet *packet) {
    dbgprint("Received DHCP acknowledge from %d.%d.%d.%d\n", packet->siaddr[0], packet->siaddr[1], packet->siaddr[2], packet->siaddr[3]);
    driver->ipv4.ip[0] = packet->yiaddr[0];
    driver->ipv4.ip[1] = packet->yiaddr[1];
    driver->ipv4.ip[2] = packet->yiaddr[2];
    driver->ipv4.ip[3] = packet->yiaddr[3];
    driver->ipv4.gateway[0] = packet->giaddr[0];
    driver->ipv4.gateway[1] = packet->giaddr[1];
    driver->ipv4.gateway[2] = packet->giaddr[2];
    driver->ipv4.gateway[3] = packet->giaddr[3];

    uint8_t *op = packet->options;
    while (op[0] != DHCP_OPTION_END) {
        if (op - packet->options >= 312) {
            dbgprint("Received DHCP packet with invalid options\n");
            return;
        }

        switch (op[0]) {
            case DHCP_OPTION_SUBNET_MASK:
                driver->ipv4.netmask[0] = op[2];
                driver->ipv4.netmask[1] = op[3];
                driver->ipv4.netmask[2] = op[4];
                driver->ipv4.netmask[3] = op[5];
                break;

            case DHCP_OPTION_ROUTER:
                driver->ipv4.gateway[0] = op[2];
                driver->ipv4.gateway[1] = op[3];
                driver->ipv4.gateway[2] = op[4];
                driver->ipv4.gateway[3] = op[5];
                break;

            case DHCP_OPTION_DNS_SERVER:
                driver->ipv4.dns[0] = op[2];
                driver->ipv4.dns[1] = op[3];
                driver->ipv4.dns[2] = op[4];
                driver->ipv4.dns[3] = op[5];
                break;

            case DHCP_OPTION_LEASE_TIME:
                driver->ipv4.lease_time = op[2] << 24 | op[3] << 16 | op[4] << 8 | op[5];
                break;
        }

        op += op[1] + 2;
    }

    dbgprint("IP: %d.%d.%d.%d\n", driver->ipv4.ip[0], driver->ipv4.ip[1], driver->ipv4.ip[2], driver->ipv4.ip[3]);
    dbgprint("Subnet: %d.%d.%d.%d\n", driver->ipv4.netmask[0], driver->ipv4.netmask[1], driver->ipv4.netmask[2], driver->ipv4.netmask[3]);
    dbgprint("Gateway: %d.%d.%d.%d\n", driver->ipv4.gateway[0], driver->ipv4.gateway[1], driver->ipv4.gateway[2], driver->ipv4.gateway[3]);
    dbgprint("DNS: %d.%d.%d.%d\n", driver->ipv4.dns[0], driver->ipv4.dns[1], driver->ipv4.dns[2], driver->ipv4.dns[3]);
    dbgprint("Lease time: %ds\n", driver->ipv4.lease_time);

    uint8_t gateway_mac[6];
    if (arp_get_mac(driver, driver->ipv4.gateway, gateway_mac, 100)) {
        dbgprint("Gateway MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", gateway_mac[0], gateway_mac[1], gateway_mac[2], gateway_mac[3], gateway_mac[4], gateway_mac[5]);
    }
}

static void dhcp_udp_listener(ethernet_driver *driver, void *data, size_t data_size) {
    dhcp_packet *packet = (dhcp_packet *) data;

    if (packet->op != DHCP_BOOT_OP_REPLY) {
        dbgprint("Received DHCP packet with invalid op code %d\n", packet->op);
        return;
    }

    uint8_t *op = packet->options;
    while (op[0] != DHCP_OPTION_END) {
        if (op - packet->options >= 312) {
            dbgprint("Received DHCP packet with invalid options\n");
            return;
        }

        if (op[0] == DHCP_OPTION_MESSAGE_TYPE) {
            switch (op[2]) {
                case DHCP_OP_OFFER:
                    dhcp_process_offer(driver, packet);
                    return;
                case DHCP_OP_ACK:
                    dhcp_process_ack(driver, packet);
                    return;
            }
        }

        op += op[1] + 2;
    }
}

void dhcp_init(ethernet_driver *driver) {
    dbgprint("dhcp_init\n");
    udp_install_listener(68, dhcp_udp_listener);
    dhcp_send_discover(driver);

    int timeout = 10000;

    for (int t = 0; t < timeout; t += 10) {
        timer_wait(10);

        if (offer_count > 0) {
            break;
        }
    }

    for (int i = 0; i < offer_count; i++) {
        uint8_t mac[6];
        if (!arp_get_mac(driver, offers[i].offered_ip, mac, 100)) {
            dbgprint("Offered IP %d.%d.%d.%d from %d.%d.%d.%d is available\n", offers[i].offered_ip[0], offers[i].offered_ip[1], offers[i].offered_ip[2], offers[i].offered_ip[3], offers[i].server_ip[0], offers[i].server_ip[1], offers[i].server_ip[2], offers[i].server_ip[3]);
            dhcp_send_request(driver, offers[i].server_ip, offers[i].offered_ip);
            break;
        } else {
            dbgprint("Offered IP %d.%d.%d.%d from %d.%d.%d.%d is not available\n", offers[i].offered_ip[0], offers[i].offered_ip[1], offers[i].offered_ip[2], offers[i].offered_ip[3], offers[i].server_ip[0], offers[i].server_ip[1], offers[i].server_ip[2], offers[i].server_ip[3]);
        }
    }
}

void dhcp_send_discover(ethernet_driver *driver) {
    dbgprint("dhcp_send_discover\n");
    dhcp_packet *packet = malloc(sizeof(dhcp_packet));
    packet->op = DHCP_BOOT_OP_REQUEST;
    packet->htype = DHCP_HTYPE_ETHERNET;
    packet->hlen = 6;
    packet->hops = 0;
    packet->xid = switch_endian_32(0x12345678);
    packet->secs = switch_endian_16(0);
    packet->flags = switch_endian_16(0);
    memset(&packet->ciaddr, 0, 4);
    memset(&packet->yiaddr, 0, 4);
    memset(&packet->siaddr, 0, 4);
    memset(&packet->giaddr, 0, 4);
    memcpy(&packet->chaddr, &driver->mac, 6);
    memset(&packet->chaddr[6], 0, 10);
    memset(&packet->sname, 0, 64);
    memset(&packet->file, 0, 128);
    packet->magic_cookie = switch_endian_32(0x63825363);
    int opt = 0;
    packet->options[opt++] = DHCP_OPTION_MESSAGE_TYPE;
    packet->options[opt++] = 1;
    packet->options[opt++] = DHCP_OP_DISCOVER;
    packet->options[opt++] = DHCP_OPTION_END;

    udp_send_packet(driver, (uint8_t[]){0, 0, 0, 0}, 68, (uint8_t[]){255, 255, 255, 255}, 67, packet, sizeof(dhcp_packet));

    free(packet);
}

void dhcp_send_request(ethernet_driver *driver, uint8_t server_ip[4], uint8_t requested_ip[4]) {
    dbgprint("dhcp_send_request\n");
    dhcp_packet *packet = malloc(sizeof(dhcp_packet));
    packet->op = DHCP_BOOT_OP_REQUEST;
    packet->htype = DHCP_HTYPE_ETHERNET;
    packet->hlen = 6;
    packet->hops = 0;
    packet->xid = switch_endian_32(0x12345678);
    packet->secs = switch_endian_16(0);
    packet->flags = switch_endian_16(0);
    memset(&packet->ciaddr, 0, 4);
    memset(&packet->yiaddr, 0, 4);
    memcpy(&packet->siaddr, server_ip, 4);
    memset(&packet->giaddr, 0, 4);
    memcpy(&packet->chaddr, &driver->mac, 6);
    memset(&packet->chaddr[6], 0, 10);
    memset(&packet->sname, 0, 64);
    memset(&packet->file, 0, 128);
    packet->magic_cookie = switch_endian_32(0x63825363);
    int opt = 0;
    packet->options[opt++] = DHCP_OPTION_MESSAGE_TYPE;
    packet->options[opt++] = 1;
    packet->options[opt++] = DHCP_OP_REQUEST;
    packet->options[opt++] = DHCP_OPTION_REQUESTED_IP;
    packet->options[opt++] = 4;
    packet->options[opt++] = requested_ip[0];
    packet->options[opt++] = requested_ip[1];
    packet->options[opt++] = requested_ip[2];
    packet->options[opt++] = requested_ip[3];
    packet->options[opt++] = DHCP_OPTION_SERVER_IDENTIFIER;
    packet->options[opt++] = 4;
    packet->options[opt++] = server_ip[0];
    packet->options[opt++] = server_ip[1];
    packet->options[opt++] = server_ip[2];
    packet->options[opt++] = server_ip[3];
    packet->options[opt++] = DHCP_OPTION_PARAMETER_REQUEST_LIST;
    packet->options[opt++] = 4;
    packet->options[opt++] = DHCP_OPTION_SUBNET_MASK;
    packet->options[opt++] = DHCP_OPTION_ROUTER;
    packet->options[opt++] = DHCP_OPTION_DOMAIN_NAME;
    packet->options[opt++] = DHCP_OPTION_DNS_SERVER;
    packet->options[opt++] = DHCP_OPTION_END;

    udp_send_packet(driver, (uint8_t[]){0, 0, 0, 0}, 68, (uint8_t[]){255, 255, 255, 255}, 67, packet, sizeof(dhcp_packet));

    free(packet);
}
