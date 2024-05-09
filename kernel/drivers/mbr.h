#ifndef MBR_H
#define MBR_H

#include <stdint.h>
#include "iodriver.h"
#include "filesystem.h"

#pragma pack(push, 1)
typedef struct mbr_partition {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sector;
    uint8_t start_cylinder;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_lba;
    uint32_t size;
} mbr_partition;
#pragma pack(pop)

filesystem *mbr_init(iodriver *driver, unsigned int partition);

filesystem *mbr_get_fs(iodriver *driver, int partition);

#endif // MBR_H
