#include "mbr.h"

#include "../debug.h"
#include "../bits.h"
#include "fs/fat.h"
#include "fs/ext2.h"
#include <string.h>

filesystem *mbr_init(iodriver *driver, unsigned int partition) {
    driver->read_sector(driver, 0, driver->io_buffer, true);

    if (driver->io_buffer[510] != 0x55 || driver->io_buffer[511] != 0xAA) {
        dbgprint("MBR signature not found: expected 0x55 0xAA, got 0x%x 0x%x\n", driver->io_buffer[510], driver->io_buffer[511]);
        return NULL;
    }

    memcpy(&partitions, &driver->io_buffer[446], sizeof(partitions));
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

    return mbr_get_fs(partition);
}

filesystem *mbr_get_fs(int partition) {
    static filesystem _fs;
    switch (partitions[partition].type) {
        case 0x00:
            return NULL;
        case 0x01: {
            _fs.type = FS_FAT12;
            _fs.start_lba = partitions[partition].start_lba;
            _fs.init = &fat_init;
            _fs.get_file_size = &fat_get_file_size;
            _fs.search_file = &fat_search_file;
            _fs.load_file = &fat_load_file;
            _fs.load_file_at = &fat_load_file_at;
            _fs.list_files = &fat_list_files;
            return &_fs;
        }
        case 0x04:
        case 0x06: {
            _fs.type = FS_FAT16;
            _fs.start_lba = partitions[partition].start_lba;
            _fs.init = &fat_init;
            _fs.get_file_size = &fat_get_file_size;
            _fs.search_file = &fat_search_file;
            _fs.load_file = &fat_load_file;
            _fs.load_file_at = &fat_load_file_at;
            _fs.list_files = &fat_list_files;
            return &_fs;
        }
        case 0x83: {
            _fs.type = FS_EXT2;
            _fs.start_lba = partitions[partition].start_lba;
            _fs.init = &ext2_init;
            _fs.get_file_size = &ext2_get_file_size;
            _fs.search_file = &ext2_search_file;
            _fs.load_file = &ext2_load_file;
            _fs.load_file_at = &ext2_load_file_at;
            _fs.list_files = &ext2_list_files;
            return &_fs;
        }
        default:
            return NULL;
    }
}
