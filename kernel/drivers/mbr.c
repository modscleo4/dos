#include "mbr.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdlib.h>
#include <string.h>
#include "../debug.h"
#include "../bits.h"
#include "fs/fat.h"
#include "fs/ext2.h"

filesystem *mbr_init(iodriver *driver, unsigned int partition) {
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
        dbgprint("\tType: %d\n", partitions[i].type);
        dbgprint("\tBootable: %d\n", ISSET_BIT_INT(partitions[i].bootable, 0x80));
    }

    return mbr_get_fs(driver, partition);
}

filesystem *mbr_get_fs(iodriver *driver, int partition) {
    mbr_partition *partitions = driver->partitions;
    filesystem *fs = malloc(sizeof(filesystem));
    switch (partitions[partition].type) {
        case 0x00:
            return NULL;
        case 0x01: {
            fs->type = FS_FAT12;
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &fat_init;
            fs->get_file_size = &fat_get_file_size;
            fs->search_file = (void *(*)(iodriver *, struct filesystem *, const char *)) &fat_search_file;
            fs->load_file = &fat_load_file;
            fs->load_file_at = &fat_load_file_at;
            fs->list_files = &fat_list_files;
            return fs;
        }
        case 0x04:
        case 0x06: {
            fs->type = FS_FAT16;
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &fat_init;
            fs->get_file_size = &fat_get_file_size;
            fs->search_file = (void *(*)(iodriver *, struct filesystem *, const char *)) &fat_search_file;
            fs->load_file = &fat_load_file;
            fs->load_file_at = &fat_load_file_at;
            fs->list_files = &fat_list_files;
            return fs;
        }
        case 0x83: {
            fs->type = FS_EXT2;
            fs->start_lba = partitions[partition].start_lba;
            fs->init = &ext2_init;
            fs->get_file_size = &ext2_get_file_size;
            fs->search_file = (void *(*)(iodriver *, struct filesystem *, const char *)) &ext2_search_file;
            fs->load_file = &ext2_load_file;
            fs->load_file_at = &ext2_load_file_at;
            fs->list_files = &ext2_list_files;
            return fs;
        }
        default:
            return NULL;
    }
}
