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
#include "../../cpu/panic.h"

static bool iso9660_read_block(iodriver *driver, filesystem *fs, uint32_t lba, uint8_t *buffer) {
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

            break;
        }

        memcpy(buffer + i, driver->io_buffer, driver->sector_size);
    }

    return true;
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

void iso9660_init(iodriver *driver, filesystem *fs) {
    if (!driver || !fs) {
        return;
    }

    if (ISO9660_SECTOR_SIZE < driver->sector_size) {
        panic("Sector size (%d) is too small for drive (%d)\n", ISO9660_SECTOR_SIZE, driver->sector_size);
    } else if (ISO9660_SECTOR_SIZE % driver->sector_size) {
        panic("Sector size (%d) is not a multiple of drive sector size (%d)\n", ISO9660_SECTOR_SIZE, driver->sector_size);
    }

    fs->case_sensitive = true;

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

                struct stat *st = calloc(1, sizeof(struct stat));
                iso9660_stat_fill(driver, fs, (iso9660_directory_entry *)fs->params, st);
                st->st_ino = 2;
                fs->rootdir = st;

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

int iso9660_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    iso9660_directory_entry *dir_entry = (iso9660_directory_entry *)st->st_private;
    struct stat dir = *st;

    if (!strcmp(path, "/")) {
        if (out_st) {
            memcpy(out_st, fs->rootdir, sizeof(struct stat));
        }

        return 0;
    }

    char *fn = strdup(path);
    char *filename = strtok(fn + 1, "/");
    while (filename && *filename) {
        dbgprint("Looking for \"%s\"\n", filename);
        if (!ISSET_BIT_INT(dir.st_mode, S_IFDIR)) {
            // Not a directory
            free(fn);
            return -ENOTDIR;
        }

        char name[256];
        for (size_t i = 0;; i++) {
            if (iso9660_readdir(driver, fs, &dir, i, name, out_st) < 0) {
                break;
            }

            if (strcmp(name, filename) == 0) {
                dir = *out_st;

                goto next;
            }
        }

        // Not found
        free(fn);
        return -ENOENT;

    next:
        filename = strtok(NULL, "/");
    }

    return 0;
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

        if (!iso9660_read_block(driver, fs, f->extent_lba + ((offset + i) / ISO9660_SECTOR_SIZE), buffer)) {
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
    if (!iso9660_read_block(driver, fs, dir_entry->extent_lba, dir)) {
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

    size_t len = entry->name_length;
    memcpy(name, entry->name, len);
    name[len] = '\0';
    if (!ISSET_BIT_INT(entry->flags, ISO9660_DIRECTORY_ENTRY_FLAG_DIRECTORY)) {
        // Remove version number
        name[len - 2] = '\0';
        name[len - 1] = '\0';
        len -= 2;
    }

    if (name[len - 1] == '.') {
        // Remove trailing dot
        name[len - 1] = '\0';
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
    out_st->st_ino = out_entry->extent_lba;

    free(dir);

    return 1;
}
