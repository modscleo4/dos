#ifndef DNS_H
#define DNS_H

#include <stdbool.h>
#include <stdint.h>
#include "../../drivers/ethernet.h"

typedef struct dns_header_flags {
    uint8_t rcode : 4;
    uint8_t z : 3;
    bool ra: 1;
    bool rd: 1;
    bool tc: 1;
    bool aa: 1;
    uint8_t opcode: 4;
    bool qr: 1;
} __attribute__((packed)) dns_header_flags;

typedef struct dns_header {
    uint16_t id;
    dns_header_flags flags;
    uint16_t questions;
    uint16_t answers;
    uint16_t authority;
    uint16_t additional;
} __attribute__((packed)) dns_header;

typedef struct dns_query {
    uint16_t type;
    uint16_t class;
} __attribute__((packed)) dns_query;

typedef struct dns_answer {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t data_length;
} __attribute__((packed)) dns_answer;

enum DNSMessage {
    DNS_MESSAGE_QUERY = 0,
    DNS_MESSAGE_RESPONSE = 1,
};

enum DNSOpcode {
    DNS_OPCODE_QUERY = 0,
    DNS_OPCODE_IQUERY = 1,
    DNS_OPCODE_STATUS = 2,
};

enum DNSRCode {
    DNS_RCODE_NO_ERROR = 0,
    DNS_RCODE_FORMAT_ERROR = 1,
    DNS_RCODE_SERVER_FAILURE = 2,
    DNS_RCODE_NAME_ERROR = 3,
    DNS_RCODE_NOT_IMPLEMENTED = 4,
    DNS_RCODE_REFUSED = 5,
    DNS_RCODE_YXDOMAIN = 6,
    DNS_RCODE_YXRRSET = 7,
    DNS_RCODE_NOTAUTH = 8,
    DNS_RCODE_NOTZONE = 9,
};

enum DNSType {
    DNS_TYPE_A = 1,
    DNS_TYPE_NS = 2,
    DNS_TYPE_CNAME = 5,
    DNS_TYPE_SOA = 6,
    DNS_TYPE_PTR = 12,
    DNS_TYPE_MX = 15,
    DNS_TYPE_TXT = 16,
    DNS_TYPE_AAAA = 28,
    DNS_TYPE_SRV = 33,
    DNS_TYPE_OPT = 41,
    DNS_TYPE_ANY = 255,
};

enum DNSClass {
    DNS_CLASS_IN = 1,
    DNS_CLASS_ANY = 255,
};

void dns_init(void);

void dns_send_query(ethernet_driver *driver, uint8_t dns_server_ip[4], const char *domain, int type);

bool dns_query_ipv4(ethernet_driver *driver, uint8_t dns_server_ip[4], const char *domain, uint8_t ip[4], int timeout);

#endif // DNS_H
