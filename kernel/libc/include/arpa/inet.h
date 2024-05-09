#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <stdint.h>

uint64_t htonll(uint64_t hostlonglong);
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint64_t ntohll(uint64_t netlonglong);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

#endif // ARPA_INET_H
