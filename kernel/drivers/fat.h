#ifndef FAT_H
#define FAT_H

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
    unsigned int bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned int reserved_sectors;
    unsigned char number_of_fat;
    unsigned int root_entries;
    unsigned int small_sectors;
    unsigned char media_type;
    unsigned int sectors_per_fat;
    unsigned int sectors_per_track;
    unsigned int num_of_heads;
    unsigned long int hidden_sectors;
    unsigned long int large_sectors;

    unsigned char disk_number;
    unsigned char current_head;
    unsigned char signature;
    unsigned long int serial_number;
    unsigned char volume_label[11];
    unsigned char filesystem[8];
} bios_params;

typedef struct fat_entry {
    unsigned char name[8];
    unsigned char ext[3];
    struct {
        unsigned char read_only: 1;
        unsigned char hidden: 1;
        unsigned char system: 1;
        unsigned char volume: 1;
        unsigned char directory: 1;
        unsigned char archive: 1;
        unsigned char device: 1;
        unsigned char lfn: 1;
    } attributes;
    unsigned char winnt: 8;
    unsigned int creation_msstamp: 8;
    struct {
        unsigned int seconds: 5;
        unsigned int minutes: 6;
        unsigned int hour: 5;
    } created_time;
    struct {
        unsigned int year: 7;
        unsigned int month: 4;
        unsigned int day: 5;
    } created_date;
    struct {
        unsigned int seconds: 5;
        unsigned int minutes: 6;
        unsigned int hour: 5;
    } last_access_date;
    unsigned int cluster_fat32: 16;
    struct {
        unsigned int hour: 5;
        unsigned int minutes: 6;
        unsigned int seconds: 5;
    } last_write_time;
    struct {
        unsigned int year: 7;
        unsigned int month: 4;
        unsigned int day: 5;
    } last_write_date;
    unsigned int cluster: 16;
    unsigned long int size: 32;
} fat_entry;

/*
 * 0x00      Entry never used
 * 0xe5      File is deleted
 * 0x2e      (A ".") Directory
 * 0x05      In a FAT32 entry, 0x05 as the lead character is translated to 0xe5, a Kanji character, so that Japanese language versions work.
 */

int fat_write();

int fat_read();

#endif //FAT_H
