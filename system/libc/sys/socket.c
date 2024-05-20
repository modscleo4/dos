#include <sys/socket.h>

#include <unistd.h>

int socket(int domain, int type, int protocol) {
    return syscall(41, domain, type, protocol);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return syscall(49, sockfd, addr, addrlen);
}
