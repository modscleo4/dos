#include "mbr.h"

#include "../debug.h"
#include "../bits.h"
#include "fat.h"
#include <string.h>

filesystem *mbr_init(iodriver *driver) {
    driver->read_sector(driver->device, 0, driver->io_buffer, true);

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

    return mbr_get_fs(0);
}

filesystem *mbr_get_fs(int partition) {
    static filesystem _fs;
    switch (partitions[partition].type) {
        case 0x00:
            return NULL;
        case 0x01: {
            _fs.type = FS_FAT12;
            _fs.init = &fat_init;
            _fs.search_file = &fat_search_file;
            _fs.load_file_at = &fat_load_file_at;
            _fs.list_files = &fat_list_files;
            return &_fs;
        }
        default:
            return NULL;
    }
}
