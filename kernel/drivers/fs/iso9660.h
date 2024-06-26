#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>
#include "../iodriver.h"
#include "../filesystem.h"

#define ISO9660_SECTOR_SIZE 2048

#pragma pack(push, 1)
typedef struct iso9660_date {
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char hundredths[2];
    uint8_t timezone;
} iso9660_date;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct iso9660_directory_date {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t timezone;
} iso9660_directory_date;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct iso9660_directory_entry {
    uint8_t length;
    uint8_t extended_attribute_length;
    uint32_t extent_lba;
    uint32_t extent_lba_be;
    uint32_t extent_size;
    uint32_t extent_size_be;
    iso9660_directory_date date;
    uint8_t flags;
    uint8_t interleave_units;
    uint8_t interleave_gap;
    uint16_t volume_sequence_number;
    uint16_t volume_sequence_number_be;
    uint8_t name_length;
    char name[1];
} iso9660_directory_entry;
#pragma pack(pop)

enum ISO9660DirectoryEntryFlags {
    ISO9660_DIRECTORY_ENTRY_FLAG_HIDDEN = 1 << 0,
    ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY = 1 << 1,
    ISO9660_DIRECTORY_ENTRY_FLAG_ASSOCIATED = 1 << 2,
    ISO9660_DIRECTORY_ENTRY_FLAG_EXTENDED = 1 << 3,
    ISO9660_DIRECTORY_ENTRY_FLAG_PERMISSIONS = 1 << 4,
    ISO9660_DIRECTORY_ENTRY_FLAG_NOT_FINAL = 1 << 7,
};

#pragma pack(push, 1)
typedef struct iso9660_path_table_entry {
    uint8_t length;
    uint8_t extended_attribute_length;
    uint32_t extent_lba;
    uint16_t parent_directory_number;
    char name[0];
} iso9660_path_table_entry;
#pragma pack(pop)

enum ISO9660VolumeDescriptorType {
    ISO9660_VOLUME_DESCRIPTOR_BOOT_RECORD = 0,
    ISO9660_VOLUME_DESCRIPTOR_PRIMARY = 1,
    ISO9660_VOLUME_DESCRIPTOR_SUPPLEMENTARY = 2,
    ISO9660_VOLUME_DESCRIPTOR_PARTITION = 3,
    ISO9660_VOLUME_DESCRIPTOR_TERMINATOR = 255,
};

#pragma pack(push, 1)
typedef struct iso9660_volume_descriptor {
    uint8_t type;
    char id[5];
    uint8_t version;
} iso9660_volume_descriptor;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct iso9660_boot_record {
    iso9660_volume_descriptor volume_descriptor;
    char system_id[32];
    char id[32];
} iso9660_boot_record;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct iso9660_primary {
    iso9660_volume_descriptor volume_descriptor;
    uint8_t unused1;
    char system_id[32];
    char volume_id[32];
    uint8_t unused2[8];
    uint32_t volume_space_size;
    uint32_t volume_space_size_be;
    uint8_t unused3[32];
    uint16_t volume_set_size;
    uint16_t volume_set_size_be;
    uint16_t volume_sequence_number;
    uint16_t volume_sequence_number_be;
    uint16_t logical_block_size;
    uint16_t logical_block_size_be;
    uint32_t path_table_size;
    uint32_t path_table_size_be;
    uint32_t type_l_path_table;
    uint32_t opt_type_l_path_table;
    uint32_t type_m_path_table;
    uint32_t opt_type_m_path_table;
    iso9660_directory_entry root_directory;
    char volume_set_id[128];
    char publisher_id[128];
    char data_preparer_id[128];
    char application_id[128];
    char copyright_file_id[37];
    char abstract_file_id[37];
    char bibliographic_file_id[37];
    iso9660_date creation_date;
    iso9660_date modification_date;
    iso9660_date expiration_date;
    iso9660_date effective_date;
    uint8_t file_structure_version;
    uint8_t unused4;
    uint8_t application_data[512];
    uint8_t unused5[653];
} iso9660_primary;
#pragma pack(pop)

void iso9660_init(iodriver *driver, filesystem *fs);

int iso9660_stat(iodriver *driver, filesystem *fs, const char *path, struct stat *st);

void *iso9660_load_file(iodriver *driver, filesystem *fs, const struct stat *st);

int iso9660_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int iso9660_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int iso9660_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

void iso9660_list_files(iodriver *driver, filesystem *fs);

#endif // ISO9660_H
