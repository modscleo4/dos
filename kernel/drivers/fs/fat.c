#include "fat.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../debug.h"
#include "../../cpu/panic.h"
#include "../../modules/bitmap.h"

static uint16_t fat12_next_cluster(unsigned int cluster, const uint8_t *buffer, unsigned int ent_offset) {
    uint16_t table_value = *(uint16_t *)&buffer[ent_offset];

    if (cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }

    return table_value;
}

static uint16_t fat16_next_cluster(unsigned int cluster, const uint8_t *buffer, unsigned int ent_offset) {
    uint16_t table_value = *(uint16_t *)&buffer[ent_offset];

    return table_value;
}

void fat_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading File Allocation Table (sector %x)...\n", fs->start_lba);

    bios_params *params = malloc(sizeof(bios_params));
    driver->read_sector(driver, fs->start_lba + 0, driver->io_buffer, true);
    memcpy(params, &driver->io_buffer[11], sizeof(bios_params));

    fs->params = params;

    char volume_label[12];
    char filesystem[9];
    strncpy(volume_label, params->volume_label, 11);
    strncpy(filesystem, params->filesystem, 8);

    printf("Volume label is %s\n", volume_label);
    printf("File system is %s\n", filesystem);
    printf("Serial number is %X\n", params->serial_number);
}

size_t fat_get_file_size(filesystem *fs, const void *_f) {
    fat_entry *f = (fat_entry *)_f;
    if (!f) {
        return 0;
    }

    return f->size;
}

const char *dos83toStr(const char *name, const char *ext) {
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

fat_entry *fat_search_file(iodriver *driver, filesystem *fs, const char *filename) {
    bios_params *params = (bios_params *)fs->params;
    fat_entry *f = malloc(sizeof(fat_entry));

    char *cmp_filename = malloc(strlen(filename) + 1);
    strcpy(cmp_filename, filename);
    strupr(cmp_filename);

    int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;
    int last_sector = params->rootdir_entries * sizeof(fat_entry);

    for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver, rootdir_sector++, driver->io_buffer, i != last_sector);
        }

        if (driver->io_buffer[i % 512] == 0) {
            continue;
        }

        memcpy(f, &driver->io_buffer[i % 512], sizeof(fat_entry));

        if (f->attributes.volume) { // Skip volume label
            continue;
        }

        if (strcmp(dos83toStr(f->name, f->ext), cmp_filename) == 0) {
            dbgprint("Found file %s at sector %d\n", filename, rootdir_sector - 1);
            return f;
        }
    }

    free(f);
    return NULL;
}

void *fat_load_file(iodriver *driver, filesystem *fs, const void *_f) {
    fat_entry *f = (fat_entry *)_f;
    if (!f) {
        return NULL;
    }

    void *addr = malloc_align(f->size, BITMAP_PAGE_SIZE);
    return fat_load_file_at(driver, fs, f, addr);
}

void *fat_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr) {
    bios_params *params = (bios_params *)fs->params;
    fat_entry *f = (fat_entry *)_f;
    if (!f) {
        return NULL;
    }

    unsigned short int cluster = f->cluster;
    unsigned int first_fat_sector = fs->start_lba + params->reserved_sectors;

    unsigned int cl = 0;
    unsigned int last_fat_sector = 0;

    uint8_t *fat_buffer = (uint8_t *) malloc(2 * params->bytes_per_sector);

    unsigned int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;

    unsigned int invalid = 0xFFFF;
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

        fat_sector = first_fat_sector + (fat_offset / params->bytes_per_sector);
        ent_offset = fat_offset % params->bytes_per_sector;
        dbgprint("fat_sector = %d\n", fat_sector);
        dbgprint("ent_offset = %d\n", ent_offset);

        unsigned int sector = (cluster - 2) * params->sectors_per_cluster + rootdir_sector + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            driver->read_sector(driver, fat_sector, fat_buffer, true);
            if (fs->type == FS_FAT12) {
                driver->read_sector(driver, fat_sector + 1, fat_buffer + params->bytes_per_sector, true);
            }
        }

        driver->read_sector(driver, sector, driver->io_buffer, true);

        memcpy(addr + cl, driver->io_buffer, params->bytes_per_sector);
        cl += params->bytes_per_sector;

        last_fat_sector = fat_sector;

        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, fat_buffer, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, fat_buffer, ent_offset);
        }

        dbgprint("Next cluster: %d\n", cluster);
    }

    free(fat_buffer);

    driver->stop(driver);

    return addr;
}

void fat_describe_file(filesystem *fs, fat_entry *f, int rootdir_sector, int level) {
    bios_params *params = (bios_params *)fs->params;
    if (f->attributes.volume) { // Skip volume label
        return;
    }

    char fname[9];
    char fext[4];
    strncpy(fname, f->name, 8);
    strncpy(fext, f->ext, 3);
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

    unsigned int file_sector = (f->cluster - 2) * params->sectors_per_cluster + rootdir_sector + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    if (f->attributes.directory) {
        printf("%-8s", fname);
        printf("    <DIR>     ");
    } else {
        printf("%-8s %-4s % 8d", fname, fext, f->size);
    }
    printf("  %04u-%02u-%02u", (f->created_date.year) + 1980, f->created_date.month, f->created_date.day);
    printf("  %02u:%02u:%02u", f->created_time.hour, f->created_time.minute, f->created_time.second);

    if (*(unsigned char *)&f->attributes != 0) {
        printf(" ");
    }

    if (f->attributes.read_only) {
        printf(" R");
    }

    if (f->attributes.hidden) {
        printf(" H");
    }

    if (f->attributes.system) {
        printf(" S");
    }

    if (f->attributes.archive) {
        printf(" A");
    }

    if (f->attributes.device) {
        printf(" D");
    }

    printf(" &%X", file_sector * params->bytes_per_sector);

    printf("\n");
}

void fat_list_files(iodriver *driver, filesystem *fs) {
    bios_params *params = (bios_params *)fs->params;
    printf("Files on disk: \n");

    fat_entry f;

    int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;
    int last_sector = params->rootdir_entries * sizeof(fat_entry);

    int sector = rootdir_sector;

    dbgprint("rootdir_sector: %d\n", rootdir_sector);
    dbgprint("last_sector: %d\n", last_sector);

    for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver, sector++, driver->io_buffer, i != last_sector);
        }

        if (driver->io_buffer[i % 512] == 0) {
            continue;
        }

        memcpy(&f, &driver->io_buffer[i % 512], sizeof(fat_entry));

        fat_describe_file(fs, &f, rootdir_sector, 0);

        if (f.attributes.directory) {
            fat_list_files_in_dir(driver, fs, &f, 1);
            driver->read_sector(driver, sector - 1, driver->io_buffer, i != last_sector);
        }
    }
}

void fat_list_files_in_dir(iodriver *driver, filesystem *fs, fat_entry *d, int level) {
    bios_params *params = (bios_params *)fs->params;
    fat_entry f;

    int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;
    int last_sector = params->rootdir_entries * sizeof(fat_entry);

    unsigned int dir_sector = (d->cluster - 2) * params->sectors_per_cluster + rootdir_sector + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

    for (int i = 0; i < 512; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            driver->read_sector(driver, dir_sector++, driver->io_buffer, i != last_sector);
        }

        if (driver->io_buffer[i % 512] == 0) {
            continue;
        }

        memcpy(&f, &driver->io_buffer[i % 512], sizeof(fat_entry));

        fat_describe_file(fs, &f, rootdir_sector, level);

        if (f.attributes.directory && i >= 2 * sizeof(fat_entry)) {
            fat_list_files_in_dir(driver, fs, &f, level + 1);
        }
    }
}
