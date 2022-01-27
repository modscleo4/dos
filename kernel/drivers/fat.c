#include "fat.h"

#include "../cpu/panic.h"
#include "../debug.h"
#include <stdio.h>
#include <string.h>

bios_params params;

static void buffer2struct(unsigned char *buffer, bios_params *p) {
    memcpy(p, &buffer[11], sizeof(bios_params));
}

static void buffer2fatentry(unsigned char *buffer, fat_entry *f) {
    memcpy(f, buffer, sizeof(fat_entry));
}

static unsigned short int fat12_next_cluster(unsigned int cluster, const unsigned char *buffer, unsigned int ent_offset) {
    unsigned short int table_value = *(unsigned short int *)&buffer[ent_offset];

    if (cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }

    return table_value;
}

static unsigned short int fat16_next_cluster(unsigned int cluster, const unsigned char *buffer, unsigned int ent_offset) {
    unsigned short int table_value = *(unsigned short int *)&buffer[ent_offset];

    return table_value;
}

void fat_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading File Allocation Table (sector %x)...\n", fs->start_lba);

    driver->read_sector(driver->device, fs->start_lba + 0, driver->io_buffer, true);

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

int fat_search_file(iodriver *driver, filesystem *fs, const char *filename, void *_f) {
    fat_entry *f = (fat_entry *)_f;
    unsigned char buffer[512];

    int rootdir_sector = fs->start_lba + params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;
    int last_sector = params.rootdir_entries * sizeof(fat_entry);

    for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver->device, rootdir_sector++, buffer, i != last_sector);
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        if (f->attributes.volume) { // Skip volume label
            continue;
        }

        if (strcmp(dos83toStr(f->name, f->ext), filename) == 0) {
            return 0;
        }
    }

    return -1;
}

void *fat_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr) {
    fat_entry *f = (fat_entry *)_f;
    if (!f) {
        return NULL;
    }

    unsigned short int cluster = f->cluster;
    unsigned int first_fat_sector = fs->start_lba + params.reserved_sectors;

    unsigned int cl = 0;
    unsigned int last_fat_sector = 0;

    unsigned char fat_buffer[512];

    unsigned int rootdir_sector = fs->start_lba + params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    unsigned int invalid;
    if (fs->type == FS_FAT12) {
        invalid = 0xFF8;
    } else if (fs->type == FS_FAT16) {
        invalid = 0xFFF8;
    }

    while (cluster < invalid) {
        unsigned int fat_offset;
        unsigned int fat_sector;
        unsigned int ent_offset;

        if (fs->type == FS_FAT12) {
            fat_offset = cluster + (cluster / 2);
        } else if (fs->type == FS_FAT16) {
            fat_offset = cluster * 2;
        }

        fat_sector = first_fat_sector + (fat_offset / params.bytes_per_sector);
        ent_offset = fat_offset % params.bytes_per_sector;

        unsigned int sector = (cluster - 2) * params.sectors_per_cluster + rootdir_sector + (params.rootdir_entries * sizeof(fat_entry) / params.bytes_per_sector);

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            driver->read_sector(driver->device, fat_sector, fat_buffer, true);
        }

        driver->read_sector(driver->device, sector, driver->io_buffer, true);

        memcpy(addr + cl, driver->io_buffer, params.bytes_per_sector);
        cl += params.bytes_per_sector;

        last_fat_sector = fat_sector;

        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, fat_buffer, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, fat_buffer, ent_offset);
        }
    }

    driver->stop(driver->device);

    return addr;
}

void fat_list_files(iodriver *driver, filesystem *fs) {
    printf("Files on disk: \n");

    fat_entry f;

    int rootdir_sector = fs->start_lba + params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;
    int last_sector = params.rootdir_entries * sizeof(fat_entry);

    for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver->device, rootdir_sector++, driver->io_buffer, i != last_sector);
        }

        if (driver->io_buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&driver->io_buffer[i % 512], &f);

        if (f.attributes.volume) { // Skip volume label
            continue;
        }

        char fname[9];
        char fext[4];
        strncpy(fname, f.name, 8);
        strncpy(fext, f.ext, 3);
        for (int i = 0; i < 8; i++) {
            if (fname[i] == ' ') {
                fname[i] = 0;
                break;
            }
        }

        for (int i = 0; i < 3; i++) {
            if (fext[i] == ' ') {
                fext[i] = 0;
                break;
            }
        }

        if (f.attributes.directory) {
            printf("%-8s", fname);
            printf("    <DIR>     ");
        } else {
            printf("%-8s %-4s % 8d", fname, fext, f.size);
        }
        printf("  %04u-%02u-%02u", (f.created_date.year) + 1980, f.created_date.month, f.created_date.day);
        printf("  %02u:%02u:%02u", f.created_time.hour, f.created_time.minute, f.created_time.second);

        if (*(unsigned char *)&f.attributes != 0) {
            printf(" ");
        }

        if (f.attributes.read_only) {
            printf(" R");
        }

        if (f.attributes.hidden) {
            printf(" H");
        }

        if (f.attributes.system) {
            printf(" S");
        }

        if (f.attributes.archive) {
            printf(" A");
        }

        if (f.attributes.device) {
            printf(" D");
        }

        printf("\n");
    }
}
