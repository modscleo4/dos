#ifndef SOCK_H
#define SOCK_H

#include "../drivers/ethernet.h"

typedef struct socket_t {
    int domain;          /* domain */
    int type;            /* type */
    int protocol;        /* protocol */
    int state;           /* state */
    int flags;           /* flags */

    size_t rx_size;      /* receive buffer size */
    ethernet_packet *rx; /* receive buffer */
    size_t tx_size;      /* transmit buffer size */
    ethernet_packet *tx; /* transmit buffer */
} socket_t;

#endif // SOCK_H
