#ifndef FAT_H
#define FAT_H

#include <stdbool.h>
#include <stdint.h>
#include "../iodriver.h"
#include "../filesystem.h"

/*
 * 0x00: Unused
 * 0xFF0-0xFF6: Reserved cluster
 * 0xFF7: Bad cluster
 * 0xFF8-0xFFF: Last cluster in a file
 * (anything else): Number of the next cluster in the file
 *
 * (physical sector number) = 33 + (FAT entry number) - 2
 */

#pragma pack(push, 1)
typedef struct bios_params {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fat;
    uint16_t rootdir_entries;
    uint16_t small_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_of_heads;
    uint32_t hidden_sectors;
    uint32_t large_sectors;

    uint8_t disk_number;
    uint8_t checkdisk;
    uint8_t signature;
    uint32_t serial_number;
    char volume_label[11];
    char filesystem[8];
} bios_params;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fat_entry_attributes {
    bool read_only : 1;
    bool hidden : 1;
    bool system : 1;
    bool volume : 1;
    bool directory : 1;
    bool archive : 1;
    bool device : 1;
    bool lfn : 1;
} fat_entry_attributes;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fat_entry_date {
    uint8_t day: 5;
    uint8_t month: 4;
    uint8_t year: 7;
} fat_entry_date;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fat_entry_time {
    uint8_t second: 5;
    uint8_t minute: 6;
    uint8_t hour: 5;
} fat_entry_time;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fat_entry {
    char name[8];
    char ext[3];
    fat_entry_attributes attributes;
    uint8_t winnt;
    uint8_t creation_msstamp;
    fat_entry_time created_time;
    fat_entry_date created_date;
    fat_entry_date last_access_date;
    uint16_t cluster_fat32;
    fat_entry_time last_write_time;
    fat_entry_date last_write_date;
    uint16_t cluster;
    uint32_t size;
} fat_entry;
#pragma pack(pop)

/*
 * 0x00      Entry never used
 * 0xe5      File is deleted
 * 0x2e      (A ".") Directory
 * 0x05      In a FAT32 entry, 0x05 as the lead character is translated to 0xe5, a Kanji character, so that Japanese language versions work.
 */

void fat_init(iodriver *driver, filesystem *fs);

int fat_stat(iodriver *driver, filesystem *fs, const char *path, struct stat *st);

void *fat_load_file(iodriver *driver, filesystem *fs, const struct stat *st);

int fat_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int fat_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int fat_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

void fat_list_files(iodriver *driver, filesystem *fs);

#endif //FAT_H
