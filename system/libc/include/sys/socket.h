#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <sys/types.h>

enum AddressFamily {
    AF_UNSPEC = 0,
    AF_INET = 2,
    AF_INET6 = 10,
};

enum SocketType {
    SOCK_STREAM = 1,
    SOCK_DGRAM = 2,
};

enum Protocol {
    IPPROTO_TCP = 6,
    IPPROTO_UDP = 17,
};

typedef unsigned short sa_family_t;

typedef unsigned int socklen_t;

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};

int socket(int domain, int type, int protocol);

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#endif // SYS_SOCKET_H
