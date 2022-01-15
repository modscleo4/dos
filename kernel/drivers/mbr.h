#ifndef MBR_H
#define MBR_H

#include "iodriver.h"
#include "filesystem.h"

enum FileSystem_Type {
    FS_EMPTY = 0,
    FS_FAT12
};

typedef struct mbr_partition {
    unsigned char bootable;
    unsigned char start_head;
    unsigned char start_sector;
    unsigned char start_cylinder;
    unsigned char type;
    unsigned char end_head;
    unsigned char end_sector;
    unsigned char end_cylinder;
    unsigned int start_lba;
    unsigned int size;
} __attribute__((packed)) mbr_partition;

mbr_partition partitions[4];

filesystem *mbr_init(iodriver *);

filesystem *mbr_get_fs(int);

#endif // MBR_H
