#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stdint.h>
#include "ip.h"
#include "tcp.h"

void http_send_request(ethernet_driver *driver, uint8_t destination_ip[4], uint16_t destination_port, const char *method, const char *path, const char *host);

bool http_receive_request(ethernet_driver *driver, void *data, size_t data_size);

#endif // HTTP_H
