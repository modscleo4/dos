#ifndef IP_H
#define IP_H

#include <stdbool.h>
#include <stdint.h>
#include "../../drivers/ethernet.h"

typedef struct ipv4_packet {
    uint8_t ihl: 4;
    uint8_t version: 4;
    uint8_t ecn: 2;
    uint8_t dscp: 6;
    uint16_t total_length;
    uint16_t identification;
    uint16_t fragment_offset: 13;
    uint8_t flags: 3;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint8_t source_ip[4];
    uint8_t destination_ip[4];
} __attribute__((packed)) ipv4_packet;

typedef struct ipv4_pseudo_header {
    uint8_t source_ip[4];
    uint8_t destination_ip[4];
    uint8_t zero;
    uint8_t protocol;
    uint16_t length;
} __attribute__((packed)) ipv4_pseudo_header;

enum IPProtocol {
    IP_PROTOCOL_HOPOPT = 0x00,
    IP_PROTOCOL_ICMP = 0x01,
    IP_PROTOCOL_IGMP = 0x02,
    IP_PROTOCOL_GGP = 0x03,
    IP_PROTOCOL_IP_IN_IP = 0x04,
    IP_PROTOCOL_ST = 0x05,
    IP_PROTOCOL_TCP = 0x06,
    IP_PROTOCOL_CBT = 0x07,
    IP_PROTOCOL_EGP = 0x08,
    IP_PROTOCOL_IGP = 0x09,
    IP_PROTOCOL_BBN_RCC_MON = 0x0A,
    IP_PROTOCOL_NVP_II = 0x0B,
    IP_PROTOCOL_PUP = 0x0C,
    IP_PROTOCOL_ARGUS = 0x0D,
    IP_PROTOCOL_EMCON = 0x0E,
    IP_PROTOCOL_XNET = 0x0F,
    IP_PROTOCOL_CHAOS = 0x10,
    IP_PROTOCOL_UDP = 0x11,
    IP_PROTOCOL_MUX = 0x12,
    IP_PROTOCOL_DCN_MEAS = 0x13,
    IP_PROTOCOL_HMP = 0x14,
    IP_PROTOCOL_PRM = 0x15,
    IP_PROTOCOL_XNS_IDP = 0x16,
    IP_PROTOCOL_TRUNK_1 = 0x17,
    IP_PROTOCOL_TRUNK_2 = 0x18,
    IP_PROTOCOL_LEAF_1 = 0x19,
    IP_PROTOCOL_LEAF_2 = 0x1A,
    IP_PROTOCOL_RDP = 0x1B,
    IP_PROTOCOL_IRTP = 0x1C,
    IP_PROTOCOL_ISO_TP4 = 0x1D,
    IP_PROTOCOL_NETBLT = 0x1E,
    IP_PROTOCOL_MFE_NSP = 0x1F,
    IP_PROTOCOL_MERIT_INP = 0x20,
    IP_PROTOCOL_DCCP = 0x21,
    IP_PROTOCOL_3PC = 0x22,
    IP_PROTOCOL_IDPR = 0x23,
    IP_PROTOCOL_XTP = 0x24,
    IP_PROTOCOL_DDP = 0x25,
    IP_PROTOCOL_IDPR_CMTP = 0x26,
    IP_PROTOCOL_TP_PLUS_PLUS = 0x27,
    IP_PROTOCOL_IL = 0x28,
    IP_PROTOCOL_IPV6 = 0x29,
    IP_PROTOCOL_SDRP = 0x2A,
    IP_PROTOCOL_IPV6_ROUTE = 0x2B,
    IP_PROTOCOL_IPV6_FRAG = 0x2C,
    IP_PROTOCOL_IDRP = 0x2D,
    IP_PROTOCOL_RSVP = 0x2E,
    IP_PROTOCOL_GRE = 0x2F,
    IP_PROTOCOL_DSR = 0x30,
    IP_PROTOCOL_BNA = 0x31,
    IP_PROTOCOL_ESP = 0x32,
    IP_PROTOCOL_AH = 0x33,
    IP_PROTOCOL_I_NLSP = 0x34,
    IP_PROTOCOL_SWIPE = 0x35,
    IP_PROTOCOL_NARP = 0x36,
    IP_PROTOCOL_MOBILE = 0x37,
    IP_PROTOCOL_TLSP = 0x38,
    IP_PROTOCOL_SKIP = 0x39,
    IP_PROTOCOL_IPV6_ICMP = 0x3A,
    IP_PROTOCOL_IPV6_NONXT = 0x3B,
    IP_PROTOCOL_IPV6_OPTS = 0x3C,

    IP_PROTOCOL_CFTP = 0x3E,

    IP_PROTOCOL_SAT_EXPAK = 0x40,
    IP_PROTOCOL_KRYPTOLAN = 0x41,
    IP_PROTOCOL_RVD = 0x42,
    IP_PROTOCOL_IPPC = 0x43,

    IP_PROTOCOL_SAT_MON = 0x45,
    IP_PROTOCOL_VISA = 0x46,
    IP_PROTOCOL_IPCV = 0x47,
    IP_PROTOCOL_CPNX = 0x48,
    IP_PROTOCOL_CPHB = 0x49,
    IP_PROTOCOL_WSN = 0x4A,
    IP_PROTOCOL_PVP = 0x4B,
    IP_PROTOCOL_BRSAT_MON = 0x4C,
    IP_PROTOCOL_SUN_ND = 0x4D,
    IP_PROTOCOL_WB_MON = 0x4E,
    IP_PROTOCOL_WB_EXPAK = 0x4F,
    IP_PROTOCOL_ISO_IP = 0x50,
    IP_PROTOCOL_VMTP = 0x51,
    IP_PROTOCOL_SECURE_VMTP = 0x52,
    IP_PROTOCOL_VINES = 0x53,
    IP_PROTOCOL_TTP = 0x54,
    IP_PROTOCOL_IPTM = 0x54,
    IP_PROTOCOL_NSFNET_IGP = 0x55,
    IP_PROTOCOL_DGP = 0x56,
    IP_PROTOCOL_TCF = 0x57,
    IP_PROTOCOL_EIGRP = 0x58,
    IP_PROTOCOL_OSPF = 0x59,
    IP_PROTOCOL_SPRITE_RPC = 0x5A,
    IP_PROTOCOL_LARP = 0x5B,
    IP_PROTOCOL_MTP = 0x5C,
    IP_PROTOCOL_AX_25 = 0x5D,
    IP_PROTOCOL_OS = 0x5E,
    IP_PROTOCOL_MICP = 0x5F,
    IP_PROTOCOL_SCC_SP = 0x60,
    IP_PROTOCOL_ETHERIP = 0x61,
    IP_PROTOCOL_ENCAP = 0x62,

    IP_PROTOCOL_GMTP = 0x64,
    IP_PROTOCOL_IFMP = 0x65,
    IP_PROTOCOL_PNNI = 0x66,
    IP_PROTOCOL_PIM = 0x67,
    IP_PROTOCOL_ARIS = 0x68,
    IP_PROTOCOL_SCPS = 0x69,
    IP_PROTOCOL_QNX = 0x6A,
    IP_PROTOCOL_A_N = 0x6B,
    IP_PROTOCOL_IPCOMP = 0x6C,
    IP_PROTOCOL_SNP = 0x6D,
    IP_PROTOCOL_COMPAQ_PEER = 0x6E,
    IP_PROTOCOL_IPX_IN_IP = 0x6F,
    IP_PROTOCOL_VRRP = 0x70,
    IP_PROTOCOL_PGM = 0x71,

    IP_PROTOCOL_L2TP = 0x73,
    IP_PROTOCOL_DDX = 0x74,
    IP_PROTOCOL_IATP = 0x75,
    IP_PROTOCOL_STP = 0x76,
    IP_PROTOCOL_SRP = 0x77,
    IP_PROTOCOL_UTI = 0x78,
    IP_PROTOCOL_SMP = 0x79,
    IP_PROTOCOL_SM = 0x7A,
    IP_PROTOCOL_PTP = 0x7B,
    IP_PROTOCOL_IS_IS_OVER_IPv4 = 0x7C,
    IP_PROTOCOL_FIRE = 0x7D,
    IP_PROTOCOL_CRTP = 0x7E,
    IP_PROTOCOL_CRUDP = 0x7F,
    IP_PROTOCOL_SSCOPMCE = 0x80,
    IP_PROTOCOL_IPLT = 0x81,
    IP_PROTOCOL_SPS = 0x82,
    IP_PROTOCOL_PIPE = 0x83,
    IP_PROTOCOL_SCTP = 0x84,
    IP_PROTOCOL_FC = 0x85,
    IP_PROTOCOL_RSVP_E2E_IGNORE = 0x86,
    IP_PROTOCOL_MOBILITY_HEADER = 0x87,
    IP_PROTOCOL_UDPLITE = 0x88,
    IP_PROTOCOL_MPLS_IN_IP = 0x89,
    IP_PROTOCOL_MANET = 0x8A,
    IP_PROTOCOL_HIP = 0x8B,
    IP_PROTOCOL_SHIM6 = 0x8C,
    IP_PROTOCOL_WESP = 0x8D,
    IP_PROTOCOL_ROHC = 0x8E,
    IP_PROTOCOL_ETHERNET = 0x8F,
};

bool ipv4_send_packet(ethernet_driver *driver, uint8_t source_ip[4], uint8_t destination_ip[4], uint8_t protocol, void *protocol_packet, size_t protocol_size, void *data, size_t data_size);

void ipv4_receive_packet(ethernet_driver *driver, ipv4_packet *packet, void *data, size_t data_size);

#endif // IP_H
