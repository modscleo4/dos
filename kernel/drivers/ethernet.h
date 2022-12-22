#ifndef ETHERNET_H
#define ETHERNET_H

#include <stddef.h>
#include <stdint.h>
#include "pci.h"

typedef struct ethernet_header {
    uint8_t destination_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
} ethernet_header;

typedef struct ethernet_packet {
    ethernet_header header;
    void *data;
} ethernet_packet;

typedef struct ipv4_config {
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    uint8_t dns[4];
} ipv4_config;

typedef struct ethernet_driver {
    unsigned int mmiobase;
    unsigned int iobase;
    uint8_t mac[6];
    ipv4_config ipv4;
    uint8_t *rx_buffer;
    unsigned int rx_buffer_size;
    unsigned int rx_tail;
    uint8_t *tx_buffer;
    unsigned int tx_buffer_size;
    unsigned int tx_tail;
    unsigned int (*write)(struct ethernet_driver *driver, ethernet_packet *packet, size_t data_size);
    int int_no;
    void (*int_handler)(struct ethernet_driver *driver);
    void (*int_enable)(struct ethernet_driver *driver);
} ethernet_driver;

enum EtherType {
    ETHERTYPE_IPV4 = 0x0800,
    ETHERTYPE_ARP = 0x0806,
    ETHERTYPE_WOL = 0x0842,
    ETHERTYPE_AVTP = 0x22F0,
    ETHERTYPE_IETF_TRILL = 0x22F3,
    ETHERTYPE_STREAM_RESERVATION = 0x22EA,
    ETHERTYPE_DEC_MOP_RC = 0x6002,
    ETHERTYPE_DECNET_PHASE_IV = 0x6003,
    ETHERTYPE_DEC_LAT = 0x6004,
    ETHERTYPE_RARP = 0x8035,
    ETHERTYPE_APPLETALK = 0x809B,
    ETHERTYPE_AARP = 0x80F3,
    ETHERTYPE_VLAN = 0x8100,
    ETHERTYPE_SLPP = 0x8102,
    ETHERTYPE_VLACP = 0x8103,
    ETHERTYPE_IPX = 0x8137,
    ETHERTYPE_QNX_QNET = 0x8204,
    ETHERTYPE_IPV6 = 0x86DD,
    ETHERTYPE_ETHERNET_FLOW_CONTROL = 0x8808,
    ETHERTYPE_LACP = 0x8809,
    ETHERTYPE_COBRA_NET = 0x8819,
    ETHERTYPE_MPLS_UNICAST = 0x8847,
    ETHERTYPE_MPLS_MULTICAST = 0x8848,
    ETHERTYPE_PPPOE_DISCOVERY = 0x8863,
    ETHERTYPE_PPPOE_SESSION = 0x8864,
    ETHERTYPE_HOMEPLUG = 0x887B,
    ETHERTYPE_EAP_OVER_LAN = 0x888E,
    ETHERTYPE_PROFINET = 0x8892,
    ETHERTYPE_HYPERSCSI = 0x889A,
    ETHERTYPE_ATA_OVER_ETHERNET = 0x88A2,
    ETHERTYPE_ETHERCAT = 0x88A4,
    ETHERTYPE_SERVICE_VLAN = 0x88A8,
    ETHERTYPE_ETHERNET_POWERLINK = 0x88AB,
    ETHERTYPE_GOOSE = 0x88B8,
    ETHERTYPE_GSE = 0x88B9,
    ETHERTYPE_SV = 0x88BA,
    ETHERTYPE_ROMON = 0x88BF,
    ETHERTYPE_LLDP = 0x88CC,
    ETHERTYPE_SERCOS_III = 0x88CD,
    ETHERTYPE_HOMEPLUG_PHY = 0x88E1,
    ETHERTYPE_MRP = 0x88E3,
    ETHERTYPE_MAC_SECURITY = 0x88E5,
    ETHERTYPE_PBB = 0x88E7,
    ETHERTYPE_PTP = 0x88F7,
    ETHERTYPE_NCSI = 0x88F8,
    ETHERTYPE_PRP = 0x88FB,
    ETHERTYPE_CFM = 0x8902,
    ETHERTYPE_FCOE = 0x8906,
    ETHERTYPE_FCOE_INIT = 0x8914,
    ETHERTYPE_TTE = 0x891D,
    ETHERTYPE_1905_1 = 0x893A,
    ETHERTYPE_HSR = 0x892F,
    ETHERTYPE_ECTP = 0x9000,
    ETHERTYPE_REDUNDANCY_TAG = 0xF1C1,
};

ethernet_driver eth[2];

void ethernet_init(pci_device *device, pci_header *header);

void ethernet_send_packet(ethernet_driver *driver, uint8_t destination_mac[6], uint16_t ethertype, void *data, size_t data_size);

void ethernet_process_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size);

#endif // ETHERNET_H
