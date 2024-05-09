#include "iso9660.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../modules/bitmap.h"

static bool iso9660_read_block(iodriver *driver, filesystem *fs, uint32_t lba, uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < ISO9660_SECTOR_SIZE; i += driver->sector_size) {
        const int MAX_TRIES = 3;

        for (int try = 1; try <= MAX_TRIES; try++) {
            if (driver->read_sector(driver, fs->start_lba + lba++ - 1, driver->io_buffer, true)) {
                dbgprint("Failed to read sector %d\n", lba);

                if (try == MAX_TRIES) {
                    return false;
                }

                continue;
            }

            memcpy(buffer + i, driver->io_buffer, size < driver->sector_size ? size : driver->sector_size);
            break;
        }
    }

    return true;
}

void iso9660_init(iodriver *driver, filesystem *fs) {
    uint8_t *buffer = malloc(ISO9660_SECTOR_SIZE);

    uint32_t sector = 0x10;
    for (int i = 0; i < 10; i++, sector++) {
        iso9660_read_block(driver, fs, sector, buffer, ISO9660_SECTOR_SIZE);

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

static void iso9660_stat_fill(iodriver *driver, filesystem *fs, const iso9660_directory_entry *f, struct stat *st) {
    if (!f) {
        return;
    }

    st->st_dev = driver->device;
    st->st_ino = 0;
    st->st_mode = S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH;
    if (ISSET_BIT_INT(f->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY)) {
        st->st_mode |= S_IFDIR;
    } else {
        st->st_mode |= S_IFREG;
    }

    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = f->extent_size;
    st->st_blksize = ISO9660_SECTOR_SIZE;
    st->st_blocks = f->extent_size / 512;
    st->st_atime = 0;
    st->st_mtime = 0;
    st->st_ctime = 0;
    st->st_private = (void *)f;
}

int iso9660_stat(iodriver *driver, filesystem *fs, const char *path, struct stat *st) {
    if (path[0] != '/') {
        // The path should be absolute
        return -EINVAL;
    }

    iso9660_directory_entry *dir_entry = (iso9660_directory_entry *)fs->params;
    iso9660_directory_entry *entry = dir_entry;
    char *cmp_filename = calloc(256, 1);

    uint8_t *dir = malloc(ISO9660_SECTOR_SIZE);

    char *fn = malloc(strlen(path) + 1);
    strcpy(fn, path);
    char *filename = strtok(fn + 1, "/");
    bool foundDir = true;
    while (filename && *filename) {
        dbgprint("Looking for %s\n", filename);
        if (!foundDir) {
            // Not a directory
            free(cmp_filename);
            free(fn);
            free(dir);
            return -ENOTDIR;
        }

        if (!iso9660_read_block(driver, fs, dir_entry->extent_lba, dir, ISO9660_SECTOR_SIZE)) {
            free(cmp_filename);
            free(fn);
            free(dir);
            return -EIO;
        }

        entry = (iso9660_directory_entry *)dir;
        if (strcmp(filename, ".") == 0) {
            filename = strtok(NULL, "/");
            foundDir = ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY);
            if (foundDir) {
                dir_entry = entry;
            }

            goto next;
        } else if (strcmp(filename, "..") == 0) {
            // Skip the entry (".")
            entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
            foundDir = ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY);
            if (foundDir) {
                dir_entry = entry;
            }

            goto next;
        }

        // Skip the first two entries (".", "..")
        entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
        entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
        while (entry->length != 0) {
            strcpy(cmp_filename, filename);
            if (!strstr(filename, ".")) {
                // Append a dot if there is no extension
                strcat(cmp_filename, ".");
            }

            if (!ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY)) {
                // Append a version number if it's a file
                strcat(cmp_filename, ";1");
            }

            if (strncmp(entry->name, cmp_filename, entry->name_length) == 0) {
                foundDir = ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY);
                if (foundDir) {
                    dir_entry = entry;
                }

                goto next;
            }

            entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
        }

        // Not found
        free(cmp_filename);
        free(fn);
        free(dir);
        return -ENOENT;

    next:
        filename = strtok(NULL, "/");
    }

    free(cmp_filename);
    free(fn);

    iso9660_directory_entry *ret = malloc(sizeof(iso9660_directory_entry));
    memcpy(ret, entry, sizeof(iso9660_directory_entry));

    free(dir);

    iso9660_stat_fill(driver, fs, ret, st);

    return 0;
}

void *iso9660_load_file(iodriver *driver, filesystem *fs, const struct stat *st) {
    const iso9660_directory_entry *f = (iso9660_directory_entry *)st->st_private;
    if (!f) {
        return NULL;
    }

    void *addr = malloc_align(f->extent_size, BITMAP_PAGE_SIZE);
    // iso9660_load_file_at(driver, fs, f, addr);
    int read_ret = 0;
    if ((read_ret = iso9660_read(driver, fs, st, addr, f->extent_size, 0)) < 0) {
        dbgprint("Failed to read file: %d\n", read_ret);
        free(addr);
        return NULL;
    }

    return addr;
}

int iso9660_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    const iso9660_directory_entry *f = (iso9660_directory_entry *)st->st_private;
    if (!f) {
        return -EBADFD;
    }

    if (offset >= f->extent_size) {
        // Offset is beyond the file size
        return 0;
    }

    if (offset + count > f->extent_size) {
        // Read only up to the file size
        count = f->extent_size - offset;
    }

    uint8_t *buffer = malloc(ISO9660_SECTOR_SIZE);

    size_t bytes_read = 0;
    for (size_t i = 0; i < f->extent_size && count; i += ISO9660_SECTOR_SIZE) {
        size_t to_read = count < ISO9660_SECTOR_SIZE ? count : ISO9660_SECTOR_SIZE;
        dbgprint("Reading %d bytes at offset %d\n", to_read, offset + i);

        if (!iso9660_read_block(driver, fs, f->extent_lba + ((offset + i) / ISO9660_SECTOR_SIZE), buffer, ISO9660_SECTOR_SIZE)) {
            free(buffer);
            return -EIO;
        }

        dbgprint("Read block %d\n", f->extent_lba + ((offset + i) / ISO9660_SECTOR_SIZE));

        size_t off = offset % ISO9660_SECTOR_SIZE;
        size_t len = off < to_read ? to_read - off : to_read;

        memcpy((uint8_t *)buf + bytes_read, buffer + off, len);
        memcpy((uint8_t *)buf + bytes_read + len, buffer + off + len, to_read - len);

        bytes_read += to_read;
        count -= to_read;
    }

    free(buffer);

    return bytes_read;
}

int iso9660_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int iso9660_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    iso9660_directory_entry *dir_entry = (iso9660_directory_entry *) st->st_private;
    uint8_t *dir = malloc(ISO9660_SECTOR_SIZE);
    if (!iso9660_read_block(driver, fs, dir_entry->extent_lba, dir, ISO9660_SECTOR_SIZE)) {
        free(dir);
        return -EIO;
    }

    iso9660_directory_entry *entry = (iso9660_directory_entry *) dir;
    for (size_t i = 0; i < index; i++) {
        if (entry->length == 0) {
            free(dir);
            return -ENOENT;
        }

        entry = (iso9660_directory_entry *)((uint8_t *)entry + entry->length);
    }

    memcpy(name, entry->name, entry->name_length);
    name[entry->name_length] = '\0';
    if (!ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY)) {
        // Remove version number
        name[entry->name_length - 2] = '\0';
        name[entry->name_length - 1] = '\0';
    } else if (name[entry->name_length - 1] == '.') {
        // Remove trailing dot
        name[entry->name_length - 1] = '\0';
    }

    if (index == 0) {
        // "." entry
        strcpy(name, ".");
    } else if (index == 1) {
        // ".." entry
        strcpy(name, "..");
    }

    iso9660_directory_entry *out_entry = malloc(sizeof(iso9660_directory_entry));
    memcpy(out_entry, entry, sizeof(iso9660_directory_entry));

    iso9660_stat_fill(driver, fs, out_entry, out_st);

    free(dir);

    return 0;
}
