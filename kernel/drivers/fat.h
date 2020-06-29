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
    unsigned char current_head;
    unsigned char signature;
    unsigned int serial_number;
    unsigned char volume_label[11];
    unsigned char filesystem[8];
} __attribute__((packed)) bios_params;

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
    } __attribute__((packed)) attributes;
    unsigned char winnt: 8;
    unsigned char creation_msstamp: 8;
    struct {
        unsigned char seconds: 5;
        unsigned char minutes: 6;
        unsigned char hour: 5;
    } __attribute__((packed)) created_time;
    struct {
        unsigned char year: 7;
        unsigned char month: 4;
        unsigned char day: 5;
    } __attribute__((packed)) created_date;
    struct {
        unsigned char seconds: 5;
        unsigned char minutes: 6;
        unsigned char hour: 5;
    } __attribute__((packed)) last_access_date;
    unsigned int cluster_fat32: 16;
    struct {
        unsigned char hour: 5;
        unsigned char minutes: 6;
        unsigned char seconds: 5;
    } __attribute__((packed)) last_write_time;
    struct {
        unsigned char year: 7;
        unsigned char month: 4;
        unsigned char day: 5;
    } __attribute__((packed)) last_write_date;
    unsigned short int cluster: 16;
    unsigned int size: 32;
} __attribute__((packed)) fat_entry;

/*
 * 0x00      Entry never used
 * 0xe5      File is deleted
 * 0x2e      (A ".") Directory
 * 0x05      In a FAT32 entry, 0x05 as the lead character is translated to 0xe5, a Kanji character, so that Japanese language versions work.
 */

void buffer2fatentry(unsigned char *, fat_entry *);

unsigned int fat_next_cluster(unsigned int, const unsigned char *, unsigned int);

int fat_writefile();

int fat_readfile();

#endif //FAT_H
