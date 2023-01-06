#include "dns.h"

#define DEBUG 1

#include <stdlib.h>
#include <string.h>
#include "udp.h"
#include "../timer.h"
#include "../../bits.h"
#include "../../debug.h"

uint16_t dns_id = 1;

static const size_t dns_ip_cache_size = 16;

typedef struct dns_ip_cache_entry {
    uint16_t id;
    char domain[256];
    uint8_t ip[4];
    uint32_t ttl;
} dns_ip_cache_entry;

static dns_ip_cache_entry *dns_ip_cache;
static size_t curr_dns_ip = 0;

static void dns_handle_response(ethernet_driver *driver, dns_header *header, size_t data_size) {
    dbgprint("dns_handle_response: %d %x\n", header->id, *(uint16_t *)&header->flags);

    if (header->flags.rcode != DNS_RCODE_NO_ERROR) {
        dbgprint("Received DNS response with error code %d, ignoring\n", header->flags.rcode);
        return;
    }

    uint16_t question_count = header->questions;
    uint16_t answer_count = header->answers;
    uint16_t authority_count = header->authority;
    uint16_t additional_count = header->additional;
    //dbgprint("Received DNS response with %d questions, %d answers, %d authority RR and %d additional RR\n", question_count, answer_count, authority_count, additional_count);

    uint8_t *data = (uint8_t *) header + sizeof(dns_header);
    for (int i = 0; i < question_count; i++) {
        while (*data) {
            uint8_t length = *data;
            data += length + 1;
        }
        data++;

        data += 2;
        data += 2;
    }

    for (int i = 0; i < answer_count; i++) {
        char domain[256];
        if (*data & 0xC0) {
            uint16_t offset = (*data & ~0xC0UL << 8UL) | *(data + 1);

            uint8_t *offset_data = (uint8_t *) header + offset;
            while (*offset_data) {
                uint8_t length = *offset_data;
                offset_data++;
                if (strlen(domain) > 0) {
                    strncat(domain, ".", 1);
                }
                strncat(domain, (char *) offset_data, length);
                offset_data += length;
            }
            data += 2;
        } else {
            while (*data) {
                uint8_t length = *data;
                data++;
                if (strlen(domain) > 0) {
                    strncat(domain, ".", 1);
                }
                strncat(domain, (char *) data, length);
                data += length;
            }
            data++;
        }

        dns_answer *answer = (dns_answer *) data;
        data += sizeof(dns_answer);

        if (switch_endian_16(answer->type) == DNS_TYPE_A) {
            uint8_t *ip = data;
            data += 4;

            dbgprint("Received DNS A response\n");
            dbgprint("Domain: %s\n", domain);
            dbgprint("IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

            for (int i = 0; i < curr_dns_ip; i++) {
                if (!strcmp(dns_ip_cache[i].domain, domain)) {
                    goto skip;
                }
            }

            if (curr_dns_ip < dns_ip_cache_size) {
                dns_ip_cache[curr_dns_ip].id = header->id;
                strcpy(dns_ip_cache[curr_dns_ip].domain, domain);
                memcpy(dns_ip_cache[curr_dns_ip].ip, ip, 4);
                dns_ip_cache[curr_dns_ip].ttl = switch_endian_32(answer->ttl);
                curr_dns_ip++;
            }

            skip: {

            }
        }

        domain[0] = 0;
    }
}

static void dns_udp_listener(ethernet_driver *driver, void *data, size_t data_size) {
    dbgprint("dns_udp_listener\n");
    dns_header *header = (dns_header *) data;

    header->id = switch_endian_16(header->id);
    *(uint16_t *)&header->flags = switch_endian_16(*(uint16_t *)&header->flags);
    header->questions = switch_endian_16(header->questions);
    header->answers = switch_endian_16(header->answers);
    header->authority = switch_endian_16(header->authority);
    header->additional = switch_endian_16(header->additional);

    if (header->flags.qr != DNS_MESSAGE_RESPONSE) {
        dbgprint("Received DNS query, ignoring\n");
        return;
    }

    dns_handle_response(driver, header, data_size);
}

void dns_init(void) {
    dbgprint("dns_init\n");
    udp_install_listener(43085, dns_udp_listener);

    dns_ip_cache = calloc(dns_ip_cache_size, sizeof(dns_ip_cache_entry));
}

void dns_send_query(ethernet_driver *driver, uint8_t dns_server_ip[4], const char *domain, int type) {
    dbgprint("dns_send_query: %d\n", dns_id);
    size_t domain_length = strlen(domain);
    dns_header *packet = malloc(sizeof(dns_header) + 1 + domain_length + 1 + sizeof(dns_query));
    packet->id = switch_endian_16(dns_id++);
    packet->flags.qr = DNS_MESSAGE_QUERY;
    packet->flags.opcode = DNS_OPCODE_QUERY;
    packet->flags.aa = 0;
    packet->flags.tc = 0;
    packet->flags.rd = 1;
    packet->flags.ra = 0;
    packet->flags.z = 0;
    packet->flags.rcode = 0;
    packet->questions = switch_endian_16(1);
    packet->answers = switch_endian_16(0);
    packet->authority = switch_endian_16(0);
    packet->additional = switch_endian_16(0);
    strncpy((uint8_t *) packet + sizeof(dns_header) + 1, domain, domain_length);
    uint8_t *domain_ptr = (uint8_t *) packet + sizeof(dns_header);
    for (int i = 0, len = 0; i <= domain_length; i++, len++) {
        if (i == domain_length || domain[i] == '.') {
            *domain_ptr = len;
            domain_ptr += len + 1;
            len = -1;
        }
    }

    dns_query *query = (dns_query *)((uint8_t *) packet + 1 + sizeof(dns_header) + domain_length + 1);
    query->type = switch_endian_16(type);
    query->class = switch_endian_16(DNS_CLASS_IN);

    udp_send_packet(driver, driver->ipv4.ip, 43085, dns_server_ip, 53, packet, sizeof(dns_header) + 1 + domain_length + 1 + 4);

    free(packet);
}

bool dns_query_ipv4(ethernet_driver *driver, uint8_t dns_server_ip[4], const char *domain, uint8_t ip[4], int timeout) {
    dbgprint("dns_query_ipv4\n");
    for (int i = 0; i < curr_dns_ip; i++) {
        if (!strcmp(dns_ip_cache[i].domain, domain)) {
            //if (dns_ip_cache[i].ttl > 0) {
                memcpy(ip, dns_ip_cache[i].ip, 4);
                return true;
            //}
        }
    }

    dns_send_query(driver, dns_server_ip, domain, DNS_TYPE_A);
    timer_wait(timeout);

    for (int i = 0; i < curr_dns_ip; i++) {
        if (!strcmp(dns_ip_cache[i].domain, domain)) {
            memcpy(ip, dns_ip_cache[i].ip, 4);
            return true;
        }
    }

    dbgprint("No DNS response received in %dms\n", timeout);

    return false;
}
