#ifndef FAT_H
#define FAT_H

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

typedef struct bios_params {
    unsigned short int bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short int reserved_sectors;
    unsigned char number_of_fat;
    unsigned short int rootdir_entries;
    unsigned short int small_sectors;
    unsigned char media_type;
    unsigned short int sectors_per_fat;
    unsigned short int sectors_per_track;
    unsigned short int num_of_heads;
    unsigned int hidden_sectors;
    unsigned int large_sectors;

    unsigned char disk_number;
    unsigned char checkdisk;
    unsigned char signature;
    unsigned int serial_number;
    unsigned char volume_label[11];
    unsigned char filesystem[8];
} __attribute__((packed)) bios_params;

typedef struct fat_entry_attributes {
    unsigned char read_only : 1;
    unsigned char hidden : 1;
    unsigned char system : 1;
    unsigned char volume : 1;
    unsigned char directory : 1;
    unsigned char archive : 1;
    unsigned char device : 1;
    unsigned char lfn : 1;
} __attribute__((packed)) fat_entry_attributes;

typedef struct fat_entry_date {
    unsigned char day: 5;
    unsigned char month: 4;
    unsigned char year: 7;
} __attribute__((packed)) fat_entry_date;

typedef struct fat_entry_time {
    unsigned char second: 5;
    unsigned char minute: 6;
    unsigned char hour: 5;
} __attribute__((packed)) fat_entry_time;

typedef struct fat_entry {
    unsigned char name[8];
    unsigned char ext[3];
    fat_entry_attributes attributes;
    unsigned char winnt : 8;
    unsigned char creation_msstamp : 8;
    fat_entry_time created_time;
    fat_entry_date created_date;
    fat_entry_date last_access_date;
    unsigned int cluster_fat32 : 16;
    fat_entry_time last_write_time;
    fat_entry_date last_write_date;
    unsigned short int cluster : 16;
    unsigned long int size : 32;
} __attribute__((packed)) fat_entry;

/*
 * 0x00      Entry never used
 * 0xe5      File is deleted
 * 0x2e      (A ".") Directory
 * 0x05      In a FAT32 entry, 0x05 as the lead character is translated to 0xe5, a Kanji character, so that Japanese language versions work.
 */

void fat_init(iodriver *driver, filesystem *fs);

unsigned long int fat_get_file_size(iodriver *driver, const void *_f);

fat_entry *fat_search_file(iodriver *driver, filesystem *fs, const char *filename);

void *fat_load_file(iodriver *driver, filesystem *fs, const void *_f);

void *fat_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr);

void fat_list_files(iodriver *driver, filesystem *fs);

void fat_list_files_in_dir(iodriver *driver, filesystem *fs, fat_entry *d, int level);

#endif //FAT_H
