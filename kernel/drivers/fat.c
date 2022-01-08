#include "fat.h"
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

void loadfat(int drive) {
    ResetFloppy(drive);
    unsigned char buffer[512];
    floppy_sector_read(drive, 0, buffer, true);

    buffer2struct(buffer, &params);
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

int fat_search_file(int drive, const char *filename, fat_entry *f) {
    unsigned char buffer[512];

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;
    int last_sector = params.rootdir_entries * sizeof(fat_entry);

    int i;
    for (i = 0; i < last_sector; i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            floppy_sector_read(drive, rootdir_sector++, buffer, i != last_sector / sizeof(fat_entry));
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        if (strcmp(dos83toStr(f->name, f->size), filename) == 0) {
            return 0;
        }
    }

    return -1;
}

void *fat_load_file_at(int drive, const fat_entry *f, void *addr) {
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

        unsigned char buffer[512];

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            floppy_sector_read(drive, fat_sector, fat_buffer, true);
        }

        floppy_sector_read(drive, sector, buffer, true);

        memcpy(addr + cl, buffer, params.bytes_per_sector);
        cl += params.bytes_per_sector;

        last_fat_sector = fat_sector;
        cluster = fat_next_cluster(cluster, fat_buffer, ent_offset);
    }

    floppy_motor_off(drive);

    return addr;
}

void listfiles(int drive) {
    printf("Files on disk: \n");

    fat_entry *f;
    unsigned char buffer[512];

    int rootdir_sector = params.reserved_sectors + params.number_of_fat * params.sectors_per_fat;

    int i;
    for (i = 0; i < params.rootdir_entries * sizeof(fat_entry); i += sizeof(fat_entry)) {
        if (i % 512 == 0) {
            floppy_sector_read(drive, rootdir_sector++, buffer, i != params.rootdir_entries * sizeof(fat_entry));
        }

        if (buffer[i % 512] == 0) {
            continue;
        }

        buffer2fatentry(&buffer[i % 512], f);

        printf("  Name: %s\n", dos83toStr(f->name, f->ext));
        printf("    Size: %d\n", f->size);
        printf("    Cluster: %d\n", f->cluster);
        printf("\n");
    }
}
