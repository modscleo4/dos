#include "fat.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/panic.h"
#include "../../modules/bitmap.h"

static uint16_t fat12_next_cluster(uint32_t cluster, const uint8_t *buffer, uint32_t ent_offset) {
    uint16_t table_value = *(uint16_t *)&buffer[ent_offset];

    if (cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }

    return table_value;
}

static uint16_t fat16_next_cluster(uint32_t cluster, const uint8_t *buffer, uint32_t ent_offset) {
    uint16_t table_value = *(uint16_t *)&buffer[ent_offset];

    return table_value;
}

static const char *dos83toStr(const char *name, const char *ext) {
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

static uint32_t fat_calculate_actual_size(filesystem *fs, const fat_entry *f) {
    bios_params *params = (bios_params *)fs->params;

    uint16_t cluster = f->cluster;
    uint32_t first_fat_sector = fs->start_lba + params->reserved_sectors;
    uint32_t size = 0;

    uint32_t invalid = 0xFFFF;
    if (fs->type == FS_FAT12) {
        invalid = 0xFF8;
    } else if (fs->type == FS_FAT16) {
        invalid = 0xFFF8;
    }

    while (cluster < invalid) {
        uint32_t fat_offset;
        uint32_t ent_offset;

        if (fs->type == FS_FAT12) {
            fat_offset = cluster + (cluster / 2);
        } else if (fs->type == FS_FAT16) {
            fat_offset = cluster * 2;
        }

        ent_offset = fat_offset % params->bytes_per_sector;
        size += params->bytes_per_sector;

        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        }
    }

    return size;
}

static void fat_stat_fill(iodriver *driver, filesystem *fs, const fat_entry *f, struct stat *st) {
    bios_params *params = (bios_params *)fs->params;

    if (!f) {
        return;
    }

    st->st_dev = driver->device;
    st->st_ino = 0;
    st->st_mode = (f->attributes.read_only ? S_IRUSR | S_IRGRP | S_IROTH : S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWGRP) | S_IXUSR | S_IXGRP | S_IXOTH;
    if (f->attributes.directory) {
        st->st_mode |= S_IFDIR;
    }

    if (f->attributes.device) {
        st->st_mode |= S_IFBLK;
    }

    if (f->attributes.archive) {
        st->st_mode |= S_IFREG;
    }

    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = f->size;
    st->st_blocks = f->size / 512 + (f->size % 512 ? 1 : 0);
    st->st_blksize = params->bytes_per_sector;
    st->st_atime = 0;
    st->st_mtime = 0;
    st->st_ctime = 0;
    st->st_private = (void *)f;
}

static bool fat_read_sector(iodriver *driver, filesystem *fs, uint32_t lba, uint8_t *buffer) {
    bios_params *params = (bios_params *)fs->params;

    for (int i = 0; i < params->bytes_per_sector; i += driver->sector_size) {
        const int MAX_TRIES = 3;

        for (int try = 1; try <= MAX_TRIES; try++) {
            if (driver->read_sector(driver, fs->start_lba + lba, driver->io_buffer, true)) {
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

void fat_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading File Allocation Table (sector %lu)...\n", fs->start_lba);

    bios_params *params = malloc(sizeof(bios_params));
    driver->read_sector(driver, fs->start_lba + 0, driver->io_buffer, true);
    memcpy(params, &driver->io_buffer[11], sizeof(bios_params));

    fs->params = params;
    fs->case_sensitive = false;

    char volume_label[12];
    char filesystem[9];
    strncpy(volume_label, params->volume_label, 11);
    strncpy(filesystem, params->filesystem, 8);

    dbgprint("Volume label is %s\n", volume_label);
    dbgprint("File system is %s\n", filesystem);
    dbgprint("Serial number is %X\n", params->serial_number);
    dbgprint("Bytes per sector: %d\n", params->bytes_per_sector);
    dbgprint("Sectors per cluster: %d\n", params->sectors_per_cluster);
    dbgprint("Reserved sectors: %d\n", params->reserved_sectors);
    dbgprint("Number of FATs: %d\n", params->number_of_fat);
    dbgprint("Sectors per FAT: %d\n", params->sectors_per_fat);
    dbgprint("Root directory entries: %d\n", params->rootdir_entries);

    if (params->bytes_per_sector < driver->sector_size) {
        panic("Sector size (%d) is too small for drive (%d)\n", params->bytes_per_sector, driver->sector_size);
    } else if (params->bytes_per_sector % driver->sector_size) {
        panic("Sector size (%d) is not a multiple of drive sector size (%d)\n", params->bytes_per_sector, driver->sector_size);
    }

    struct stat *st = calloc(1, sizeof(struct stat));
    fat_entry *f = malloc(sizeof(fat_entry));
    memset(f, 0, sizeof(fat_entry));
    f->attributes.directory = true;
    f->cluster = 0;
    f->size = params->rootdir_entries * sizeof(fat_entry);
    fat_stat_fill(driver, fs, f, st);
    st->st_ino = f->cluster; // Root directory "inode"
    fs->rootdir = st;

    // Cache the entire FAT in memory
    dbgprint("Caching FAT...\n");
    fs->bitmap = calloc(params->sectors_per_fat, params->bytes_per_sector);
    for (int fat = 0; fat < params->number_of_fat; fat++) {
        bool failed = false;

        for (int i = 0; i < params->sectors_per_fat; i++) {
            if (driver->read_sector(driver, fs->start_lba + params->reserved_sectors + (fat * params->sectors_per_fat * params->bytes_per_sector) + i, driver->io_buffer, true)) {
                failed = true;
                break;
            }

            memcpy((uint8_t *)fs->bitmap + i * params->bytes_per_sector, driver->io_buffer, params->bytes_per_sector);
        }

        dbgprint("FAT %d %s\n", fat, failed ? "failed" : "succeeded");

        if (!failed) {
            break;
        }
    }
}

int fat_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    bios_params *params = (bios_params *)fs->params;
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

        char name[13];
        for (size_t i = 0;; i++) {
            if (fat_readdir(driver, fs, &dir, i, name, out_st) < 0) {
                break;
            }

            if (stricmp(name, filename) == 0) {
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

int fat_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    bios_params *params = (bios_params *)fs->params;
    fat_entry *f = (fat_entry *)st->st_private;
    if (!f) {
        return -EBADFD;
    }

    if (offset >= f->size) {
        // Offset is beyond the file size
        return 0;
    }

    if (offset + count > f->size) {
        // Read only up to the file size
        count = f->size - offset;
    }

    dbgprint("Reading %lu bytes from offset %lu\n", count, offset);

    uint16_t cluster = f->cluster;
    uint32_t first_fat_sector = fs->start_lba + params->reserved_sectors;
    dbgprint("First FAT sector: %lu (%lu + %u)\n", first_fat_sector, fs->start_lba, params->reserved_sectors);

    size_t bytes_read = 0;

    uint32_t rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;

    uint32_t invalid = 0xFFFF;
    if (fs->type == FS_FAT12) {
        invalid = 0xFF8;
    } else if (fs->type == FS_FAT16) {
        invalid = 0xFFF8;
    }

    // Skip clusters until we reach the offset
    while (offset >= params->bytes_per_sector && cluster < invalid) {
        uint32_t fat_offset;
        uint32_t fat_sector;
        uint32_t ent_offset;

        if (fs->type == FS_FAT12) {
            fat_offset = cluster + (cluster / 2);
        } else if (fs->type == FS_FAT16) {
            fat_offset = cluster * 2;
        }

        fat_sector = first_fat_sector + (fat_offset / params->bytes_per_sector);
        ent_offset = fat_offset % params->bytes_per_sector;
        dbgprint("fat_offset = %lu, fat_sector = %lu, ent_offset = %lu\n", fat_offset, fat_sector, ent_offset);

        offset -= params->bytes_per_sector;
        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        }
    }

    while (cluster < invalid && count) {
        size_t to_read = count > params->bytes_per_sector ? params->bytes_per_sector : count;
        uint32_t fat_offset;
        uint32_t fat_sector;
        uint32_t ent_offset;

        if (fs->type == FS_FAT12) {
            fat_offset = cluster + (cluster / 2);
        } else if (fs->type == FS_FAT16) {
            fat_offset = cluster * 2;
        }

        fat_sector = first_fat_sector + (fat_offset / params->bytes_per_sector);
        ent_offset = fat_offset % params->bytes_per_sector;
        dbgprint("fat_offset = %lu, fat_sector = %lu, ent_offset = %lu\n", fat_offset, fat_sector, ent_offset);

        uint32_t sector = (cluster - 2) * params->sectors_per_cluster + rootdir_sector + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

        if (driver->read_sector(driver, sector, driver->io_buffer, true)) {
            return -EIO;
        }

        size_t off = offset % params->bytes_per_sector;
        size_t len = off < to_read ? to_read - off : to_read;

        memcpy((uint8_t *)buf + bytes_read, driver->io_buffer + off, len);
        memcpy((uint8_t *)buf + bytes_read + len, driver->io_buffer + off + len, to_read - len);

        bytes_read += to_read;

        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, (uint8_t *)fs->bitmap + (fat_offset / params->bytes_per_sector) * params->bytes_per_sector, ent_offset);
        }

        dbgprint("Next cluster: %d\n", cluster);
        count -= to_read;
    }

    return bytes_read;
}

int fat_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int fat_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    bios_params *params = (bios_params *)fs->params;
    fat_entry *f = (fat_entry *)st->st_private;
    if (!f) {
        return -EBADFD;
    }

    if (!f->attributes.directory) {
        return -ENOTDIR;
    }

    int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;

    fat_entry *entry = malloc(sizeof(fat_entry));
    int dir_sector = rootdir_sector;
    if (f->cluster == 0) { // Root directory
        int i = 0;
        for (int j = 0; j < f->size; j += sizeof(fat_entry)) {
            if (j % params->bytes_per_sector == 0) {
                if (driver->read_sector(driver, dir_sector++, driver->io_buffer, true)) {
                    free(entry);
                    return -EIO;
                }
            }

            if (driver->io_buffer[j % params->bytes_per_sector] == 0) {
                continue;
            }

            memcpy(entry, &driver->io_buffer[j % params->bytes_per_sector], sizeof(fat_entry));

            if (entry->attributes.volume) { // Skip volume label
                continue;
            }

            if (i == index) {
                if (entry->attributes.directory) {
                    entry->size = fat_calculate_actual_size(fs, entry);
                }

                strncpy(name, dos83toStr(entry->name, entry->ext), 13);
                fat_stat_fill(driver, fs, entry, out_st);
                out_st->st_ino = entry->cluster; // Inode is the cluster number
                return 0;
            }

            i++;
        }

        return -ENOENT;
    }

    size_t i = 0;
    size_t j = 0;
    uint8_t *block_buffer = calloc(1, params->bytes_per_sector);
    while (fat_read(driver, fs, st, block_buffer, 1 * params->bytes_per_sector, i) > 0) {
        for (size_t offset = 0; offset < params->bytes_per_sector; offset += sizeof(fat_entry)) {
            if (block_buffer[offset] == 0) {
                break;
            }

            memcpy(entry, block_buffer + offset, sizeof(fat_entry));

            if (j == index) {
                if (entry->attributes.directory) {
                    entry->size = fat_calculate_actual_size(fs, entry);
                }

                strncpy(name, dos83toStr(entry->name, entry->ext), 13);
                fat_stat_fill(driver, fs, entry, out_st);
                out_st->st_ino = entry->cluster; // Inode is the cluster number

                free(block_buffer);

                return 1;
            }

            j++;
        }

        i += params->bytes_per_sector;
        memset(block_buffer, 0, params->bytes_per_sector);
    }

    free(block_buffer);

    return -ENOENT;
}
