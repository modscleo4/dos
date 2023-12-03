#include "iso9660.h"

#define DEBUG 1
#define DEBUG_SERIAL 1
#include <stdlib.h>
#include <string.h>
#include "../../debug.h"
#include "../../modules/bitmap.h"

static bool iso9660_read_block(iodriver *driver, filesystem *fs, uint32_t lba, uint8_t *buffer) {
    for (size_t i = 0; i < ISO9660_SECTOR_SIZE; i += driver->sector_size) {
        if (driver->read_sector(driver, fs->start_lba + lba++ - 1, buffer + i, true)) {
            dbgprint("Failed to read sector %d\n", lba);
            return false;
        }
    }

    return true;
}

void iso9660_init(iodriver *driver, filesystem *fs) {
    uint8_t *buffer = malloc(ISO9660_SECTOR_SIZE);

    uint32_t sector = 0x10;
    for (int i = 0; i < 10; i++, sector++) {
        iso9660_read_block(driver, fs, sector, buffer);

        iso9660_volume_descriptor *vol = (iso9660_volume_descriptor *) buffer;
        switch (vol->type) {
            case ISO9660_VOLUME_DESCRIPTOR_BOOT_RECORD: {
                iso9660_boot_record *boot = (iso9660_boot_record *) buffer;
                dbgprint("Boot record\n");

                dbgprint("\tSystem identifier: %.32s\n", boot->system_id);
                dbgprint("\tVolume identifier: %.32s\n", boot->id);
                break;
            }
            case ISO9660_VOLUME_DESCRIPTOR_PRIMARY: {
                iso9660_primary *primary = (iso9660_primary *) buffer;
                dbgprint("Primary volume descriptor\n");

                dbgprint("\tSystem identifier: %.32s\n", primary->system_id);
                dbgprint("\tVolume identifier: %.32s\n", primary->volume_id);
                dbgprint("\tVolume space size: %d\n", primary->volume_space_size);
                dbgprint("\tVolume set size: %d\n", primary->volume_set_size);
                dbgprint("\tVolume sequence number: %d\n", primary->volume_sequence_number);
                dbgprint("\tLogical block size: %d\n", primary->logical_block_size);
                dbgprint("\tPath table size: %d\n", primary->path_table_size);
                dbgprint("\tType L path table location: %d\n", primary->type_l_path_table);

                fs->params = malloc(sizeof(iso9660_directory_entry));
                memcpy(fs->params, &primary->root_directory, sizeof(iso9660_directory_entry));
                break;
            }
            case ISO9660_VOLUME_DESCRIPTOR_SUPPLEMENTARY:
                dbgprint("Supplementary volume descriptor\n");
                break;
            case ISO9660_VOLUME_DESCRIPTOR_PARTITION:
                dbgprint("Partition volume descriptor\n");
                break;
            case ISO9660_VOLUME_DESCRIPTOR_TERMINATOR:
                dbgprint("Volume descriptor set terminator\n");
                goto stop;
            default:
                dbgprint("Unknown volume descriptor type: %d\n", vol->type);
                break;
        }
    }

stop:
    free(buffer);
}

size_t iso9660_get_file_size(filesystem *fs, const void *_f) {
    const iso9660_directory_entry *f = (iso9660_directory_entry *) _f;
    if (!f) {
        return 0;
    }

    return f->extent_size;
}

iso9660_directory_entry *iso9660_search_file(iodriver *driver, filesystem *fs, const char *filename) {
    iso9660_directory_entry *dir_entry = (iso9660_directory_entry *)fs->params;
    char *cmp_filename = calloc(256, 1);
    strcpy(cmp_filename, filename);
    strcat(cmp_filename, ";1");

    uint8_t *dir = malloc(dir_entry->length);
    for (size_t i = 0; i < dir_entry->length; i += ISO9660_SECTOR_SIZE) {
        iso9660_read_block(driver, fs, dir_entry->extent_lba + (i / ISO9660_SECTOR_SIZE), dir + i);
    }

    iso9660_directory_entry *entry = (iso9660_directory_entry *)dir;
    while (entry->length != 0) {
        entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
        if (strcmp(entry->name, cmp_filename) == 0) {
            free(cmp_filename);
            return entry;
        }
    }

    free(cmp_filename);
    return NULL;
}

void *iso9660_load_file(iodriver *driver, filesystem *fs, const void *_f) {
    const iso9660_directory_entry *f = (iso9660_directory_entry *) _f;
    if (!f) {
        return NULL;
    }

    void *addr = malloc_align(f->extent_size, BITMAP_PAGE_SIZE);
    return iso9660_load_file_at(driver, fs, f, addr);
}

void *iso9660_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr) {
    const iso9660_directory_entry *f = (iso9660_directory_entry *)_f;
    if (!f) {
        return NULL;
    }

    for (size_t i = 0; i < f->extent_size; i += ISO9660_SECTOR_SIZE) {
        iso9660_read_block(driver, fs, f->extent_lba + (i / ISO9660_SECTOR_SIZE), addr + i);
    }

    return addr;
}

static void iso9660_list_files_in_dir(iodriver *driver, filesystem *fs, iso9660_directory_entry *dir_entry, int level) {
    dbgprint("Directory: %.*s\n", dir_entry->name_length, dir_entry->name);

    uint8_t *dir = malloc(dir_entry->length);
    for (size_t i = 0; i < dir_entry->length; i += ISO9660_SECTOR_SIZE) {
        iso9660_read_block(driver, fs, dir_entry->extent_lba + (i / ISO9660_SECTOR_SIZE), dir + i);
    }

    iso9660_directory_entry *entry = (iso9660_directory_entry *) dir;
    while (entry->length != 0) {
        dbgprint("\tFile: %.*s\n", entry->name_length, entry->name);
        entry = (iso9660_directory_entry *) ((uint8_t *) entry + entry->length);
    }
}

void iso9660_list_files(iodriver *driver, filesystem *fs) {
    iso9660_directory_entry *dir_entry = (iso9660_directory_entry *) fs->params;
    iso9660_list_files_in_dir(driver, fs, dir_entry, 0);
}
