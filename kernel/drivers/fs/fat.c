#include "fat.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void fat_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading File Allocation Table (sector %x)...\n", fs->start_lba);

    bios_params *params = malloc(sizeof(bios_params));
    driver->read_sector(driver, fs->start_lba + 0, driver->io_buffer, true);
    memcpy(params, &driver->io_buffer[11], sizeof(bios_params));

    fs->params = params;

    char volume_label[12];
    char filesystem[9];
    strncpy(volume_label, params->volume_label, 11);
    strncpy(filesystem, params->filesystem, 8);

    printf("Volume label is %s\n", volume_label);
    printf("File system is %s\n", filesystem);
    printf("Serial number is %X\n", params->serial_number);
}

static void fat_stat_fill(iodriver *driver, filesystem *fs, const fat_entry *f, struct stat *st) {
    if (!f) {
        return;
    }

    st->st_dev = driver->device;
    st->st_ino = 0;
    st->st_mode = (f->attributes.read_only ? S_IRUSR | S_IRGRP | S_IROTH : S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWGRP) | S_IXUSR | S_IXGRP | S_IXOTH;
    if (f->attributes.directory) {
        st->st_mode |= S_IFDIR;
    } else if (f->attributes.device) {
        st->st_mode |= S_IFBLK;
    } else if (f->attributes.archive) {
        st->st_mode |= S_IFREG;
    }

    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = f->size;
    st->st_blocks = f->size / 512 + (f->size % 512 ? 1 : 0);
    st->st_blksize = 512;
    st->st_atime = 0;
    st->st_mtime = 0;
    st->st_ctime = 0;
    st->st_private = (void *)f;
}

int fat_stat(iodriver *driver, filesystem *fs, const char *path, struct stat *st) {
    if (path[0] != '/') {
        // The path should be absolute
        return -EINVAL;
    }

    bios_params *params = (bios_params *)fs->params;
    fat_entry *f = malloc(sizeof(fat_entry));
    memset(f, 0, sizeof(fat_entry));
    f->attributes.directory = true;
    f->cluster = 2;
    f->size = 0;

    int rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;
    int last_sector = params->rootdir_entries * sizeof(fat_entry);

    char *fn = malloc(strlen(path) + 1);
    strcpy(fn, path);
    char *filename = strtok(fn + 1, "/");
    bool foundDir = true;
    while (filename && *filename) {
        dbgprint("Looking for %s\n", filename);
        if (!foundDir) {
            // Not a directory
            free(f);
            free(fn);
            return -ENOTDIR;
        }

        strupr(filename);

        for (int i = 0; i < last_sector; i += sizeof(fat_entry)) {
            if (i % 512 == 0) {
                dbgprint("Reading sector %d\n", rootdir_sector);
                if (driver->read_sector(driver, rootdir_sector++, driver->io_buffer, i != last_sector)) {
                    return -EIO;
                }
            }

            if (driver->io_buffer[i % 512] == 0) {
                continue;
            }

            memcpy(f, &driver->io_buffer[i % 512], sizeof(fat_entry));

            if (f->attributes.volume) { // Skip volume label
                continue;
            }

            if (strcmp(dos83toStr(f->name, f->ext), filename) == 0) {
                foundDir = f->attributes.directory;
                if (foundDir) {
                    rootdir_sector = fs->start_lba + params->reserved_sectors + params->number_of_fat * params->sectors_per_fat;
                    rootdir_sector += (f->cluster - 2) * params->sectors_per_cluster + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);
                }

                goto next;
            }
        }

        // Not found
        free(f);
        free(fn);
        return -ENOENT;

    next:
        filename = strtok(NULL, "/");
    }

    fat_stat_fill(driver, fs, f, st);

    return 0;
}

void *fat_load_file(iodriver *driver, filesystem *fs, const struct stat *st) {
    fat_entry *f = (fat_entry *)st->st_private;
    if (!f) {
        return NULL;
    }

    void *addr = malloc_align(f->size, BITMAP_PAGE_SIZE);
    //fat_load_file_at(driver, fs, st, addr);
    int read_ret = 0;
    if ((read_ret = fat_read(driver, fs, st, addr, f->size, 0)) < 0) {
        dbgprint("Failed to read file: %d\n", read_ret);
        free(addr);
        return NULL;
    }

    return addr;
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

    uint16_t cluster = f->cluster;
    uint32_t first_fat_sector = fs->start_lba + params->reserved_sectors;

    size_t bytes_read = 0;
    uint32_t last_fat_sector = 0;

    uint8_t *fat_buffer = (uint8_t *)malloc(2 * params->bytes_per_sector);

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
        dbgprint("fat_sector = %uld, ent_offset = %uld\n", ent_offset);

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            if (driver->read_sector(driver, fat_sector, driver->io_buffer, true)) {
                free(fat_buffer);
                return -EIO;
            }

            memcpy(fat_buffer, driver->io_buffer, params->bytes_per_sector);
            if (fs->type == FS_FAT12) {
                if (driver->read_sector(driver, fat_sector + 1, driver->io_buffer, true)) {
                    free(fat_buffer);
                    return -EIO;
                }

                memcpy(fat_buffer + params->bytes_per_sector, driver->io_buffer, params->bytes_per_sector);
            }
        }

        offset -= params->bytes_per_sector;
        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, fat_buffer, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, fat_buffer, ent_offset);
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
        dbgprint("fat_sector = %d, ent_offset = %d\n", ent_offset);

        uint32_t sector = (cluster - 2) * params->sectors_per_cluster + rootdir_sector + (params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

        if (!last_fat_sector || last_fat_sector != fat_sector) {
            if (driver->read_sector(driver, fat_sector, driver->io_buffer, true)) {
                free(fat_buffer);
                return -EIO;
            }

            memcpy(fat_buffer, driver->io_buffer, params->bytes_per_sector);
            if (fs->type == FS_FAT12) {
                if (driver->read_sector(driver, fat_sector + 1, driver->io_buffer, true)) {
                    free(fat_buffer);
                    return -EIO;
                }

                memcpy(fat_buffer + params->bytes_per_sector, driver->io_buffer, params->bytes_per_sector);
            }
        }

        if (driver->read_sector(driver, sector, driver->io_buffer, true)) {
            free(fat_buffer);
            return -EIO;
        }

        size_t off = offset % params->bytes_per_sector;
        size_t len = off < to_read ? to_read - off : to_read;

        memcpy((uint8_t *)buf + bytes_read, driver->io_buffer + off, len);
        memcpy((uint8_t *)buf + bytes_read + len, driver->io_buffer + off + len, to_read - len);

        bytes_read += to_read;

        last_fat_sector = fat_sector;

        if (fs->type == FS_FAT12) {
            cluster = fat12_next_cluster(cluster, fat_buffer, ent_offset);
        } else if (fs->type == FS_FAT16) {
            cluster = fat16_next_cluster(cluster, fat_buffer, ent_offset);
        }

        dbgprint("Next cluster: %d\n", cluster);
        count -= to_read;
    }

    free(fat_buffer);

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
    int last_sector = params->rootdir_entries * sizeof(fat_entry);

    fat_entry *dir = malloc(sizeof(fat_entry));
    int dir_sector = (f->cluster - 2) * params->sectors_per_cluster + rootdir_sector + (f->name[0] == 0 ? 0 : params->rootdir_entries * sizeof(fat_entry) / params->bytes_per_sector);

    int i = 0;
    for (int j = 0; j < 512; j += sizeof(fat_entry)) {
        if (j % 512 == 0) {
            if (driver->read_sector(driver, dir_sector++, driver->io_buffer, j != last_sector)) {
                free(dir);
                return -EIO;
            }
        }

        if (driver->io_buffer[j % 512] == 0) {
            continue;
        }

        memcpy(dir, &driver->io_buffer[j % 512], sizeof(fat_entry));

        if (dir->attributes.volume) { // Skip volume label
            continue;
        }

        if (i == index) {
            strncpy(name, dos83toStr(dir->name, dir->ext), 13);
            fat_stat_fill(driver, fs, dir, out_st);
            return 0;
        }

        i++;
    }

    return -ENOENT;
}
