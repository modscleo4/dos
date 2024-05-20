#include "mbr.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include "../debug.h"
#include "../bits.h"
#include "fs/fat.h"
#include "fs/ext2.h"
#include "fs/iso9660.h"

filesystem *mbr_init(iodriver *driver, unsigned int partition) {
    dbgprint("Reading MBR %p\n", driver->read_sector);
    driver->read_sector(driver, 0, driver->io_buffer, true);

    if (driver->io_buffer[510] != 0x55 || driver->io_buffer[511] != 0xAA) {
        dbgprint("MBR signature not found: expected 0x55 0xAA, got 0x%x 0x%x\n", driver->io_buffer[510], driver->io_buffer[511]);
        return NULL;
    }

    mbr_partition *partitions = calloc(4, sizeof(mbr_partition));
    driver->partitions = partitions;
    memcpy(driver->partitions, &driver->io_buffer[446], 4 * sizeof(mbr_partition));

    for (int i = 0; i < 4; i++) {
        if (partitions[i].start_lba == 0) {
            dbgprint("<Partition %d is empty>\n", i);
            continue;
        }

        dbgprint("Partition %d\n", i);

        dbgprint("\tStart: %d (%d %d %d)\n", partitions[i].start_lba, partitions[i].start_cylinder, partitions[i].start_head, partitions[i].start_sector);
        dbgprint("\tEnd: %d (%d %d %d)\n", partitions[i].start_lba + partitions[i].size, partitions[i].end_cylinder, partitions[i].end_head, partitions[i].end_sector);
        dbgprint("\tType: %X\n", partitions[i].type);
        dbgprint("\tBootable: %c\n", ISSET_BIT_INT(partitions[i].bootable, 0x80) ? 'Y' : 'N');
    }

    return mbr_get_fs(driver, partition);
}

filesystem *mbr_get_fs(iodriver *driver, int partition) {
    mbr_partition *partitions = driver->partitions;
    filesystem *fs = malloc(sizeof(filesystem));
    switch (partitions[partition].type) {
        case 0x00:
            free(fs);
            return NULL;
        case 0x01: {
            fs->type = FS_FAT12;
            fs->name = "fat12";
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &fat_init;
            fs->stat = &fat_stat;
            fs->read = &fat_read;
            fs->write = &fat_write;
            fs->readdir = &fat_readdir;
            return fs;
        }
        case 0x04:
        case 0x06: {
            fs->type = FS_FAT16;
            fs->name = "fat16";
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &fat_init;
            fs->stat = &fat_stat;
            fs->read = &fat_read;
            fs->write = &fat_write;
            fs->readdir = &fat_readdir;
            return fs;
        }
        case 0x83: {
            fs->type = FS_EXT2;
            fs->name = "ext2";
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &ext2_init;
            fs->stat = &ext2_stat;
            fs->read = &ext2_read;
            fs->write = &ext2_write;
            fs->readdir = &ext2_readdir;
            return fs;
        }
        case 0x96:
        case 0xCD:
            fs->type = FS_ISO9660;
            fs->name = "iso9660";
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &iso9660_init;
            fs->stat = &iso9660_stat;
            fs->read = &iso9660_read;
            fs->write = &iso9660_write;
            fs->readdir = &iso9660_readdir;
            return fs;
        default:
            free(fs);
            return NULL;
    }
}
