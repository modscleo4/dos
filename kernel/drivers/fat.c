#include "fat.h"

#include "../cpu/panic.h"
#include "../debug.h"
#include <stdio.h>
#include <string.h>

bios_params params;

void buffer2struct(unsigned char *buffer, bios_params *p) {
    memcpy(p, &buffer[11], sizeof(bios_params));
}

void buffer2fatentry(unsigned char *buffer, fat_entry *f) {
    memcpy(f, buffer, sizeof(fat_entry));
}

unsigned short int fat_next_cluster(unsigned int cluster, const unsigned char *buffer, unsigned int ent_offset) {
    unsigned short int table_value = *(unsigned short int *)&buffer[ent_offset];

    if (cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }

    return table_value;
}

void fat_init(iodriver *driver) {
    dbgprint("Reading File Allocation Table...\n");

    driver->read_sector(driver->device, 0, driver->io_buffer, true);

    buffer2struct(driver->io_buffer, &params);
    char volume_label[12];
    char filesystem[9];
    strncpy(volume_label, params.volume_label, 11);
    strncpy(filesystem, params.filesystem, 8);

    printf("Volume label is %s\n", volume_label);
    printf("File system is %s\n", filesystem);
    printf("Serial number is %X\n", params.serial_number);
}

char *dos83toStr(const char *name, const char *ext) {
    static char ret[13];

    strncpy(ret, name, 8);
    int _i;
    for (_i = 0; _i < 8; _i++) {
        if (!ret[_i]) {
            break;
        } else if (ret[_i] == ' ') {
            ret[_i] = 0;
            break;
        }
    }

    if (ext[0] != ' ' || ext[1] != ' ' || ext[2] != ' ') {
        strncat(ret + _i, ".", 1);
        strncat(ret + _i + 1, ext, 3);
    }

    return ret;
}

int fat_search_file(iodriver *driver, const char *filename, void *_f) {
    fat_entry *f = (fat_entry *)_f;
    unsigned char buffer[512];

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;
    int last_sector = params.rootdir_entries * sizeof(fat_entry);

    for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver->device, rootdir_sector++, buffer, i != last_sector / sizeof(fat_entry));
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        if (strcmp(dos83toStr(f->name, f->ext), filename) == 0) {
            return 0;
        }
    }

    return -1;
}

void *fat_load_file_at(iodriver *driver, const void *_f, void *addr) {
    fat_entry *f = (fat_entry *)_f;
    if (!f) {
        return NULL;
    }

    unsigned short int cluster = f->cluster;
    unsigned int first_fat_sector = params.reserved_sectors;

    unsigned int cl = 0;
    unsigned int last_fat_sector = 0;

    unsigned char fat_buffer[512];

    unsigned int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    while (cluster < 0xFF8) {
        unsigned int fat_offset = cluster + (cluster / 2);
        unsigned int fat_sector = first_fat_sector + (fat_offset / params.bytes_per_sector);
        unsigned int ent_offset = fat_offset % params.bytes_per_sector;

        unsigned int sector = (cluster - 2) * params.sectors_per_cluster + rootdir_sector + (params.rootdir_entries * sizeof(fat_entry) / params.bytes_per_sector);

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            driver->read_sector(driver->device, fat_sector, fat_buffer, true);
        }

        driver->read_sector(driver->device, sector, driver->io_buffer, true);

        memcpy(addr + cl, driver->io_buffer, params.bytes_per_sector);
        cl += params.bytes_per_sector;

        last_fat_sector = fat_sector;
        cluster = fat_next_cluster(cluster, fat_buffer, ent_offset);
    }

    driver->stop(driver->device);

    return addr;
}

void fat_list_files(iodriver *driver) {
    printf("Files on disk: \n");

    fat_entry f;

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    for (int i = 0; i < params.rootdir_entries * sizeof(fat_entry); i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver->device, rootdir_sector++, driver->io_buffer, i != params.rootdir_entries * sizeof(fat_entry));
        }

        if (driver->io_buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&driver->io_buffer[i % 512], &f);

        printf("  Name: %s\n", dos83toStr(f.name, f.ext));
        printf("    Size: %d\n", f.size);
        printf("    Cluster: %d\n", f.cluster);
        printf("\n");
    }
}
