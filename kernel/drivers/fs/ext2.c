#include "ext2.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/panic.h"
#include "../../drivers/serial.h"
#include "../../modules/bitmap.h"

static bool ext2_read_block(iodriver *driver, filesystem *fs, uint32_t lba, uint8_t *buffer) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    for (int i = 0; i < block_size; i += driver->sector_size) {
        const int MAX_TRIES = 3;

        for (int try = 1; try <= MAX_TRIES; try++) {
            if (driver->read_sector(driver, fs->start_lba + lba * block_size / driver->sector_size + i / driver->sector_size, driver->io_buffer, true)) {
                dbgprint("Failed to read block %d\n", lba);

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

static int ext2_read_bgd(iodriver *driver, filesystem *fs, ext2_block_group_descriptor *bgd, int index) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    uint8_t *block_buffer = malloc(block_size);

    if (!ext2_read_block(driver, fs, bgd_block, block_buffer)) {
        free(block_buffer);
        return -EIO;
    }

    memcpy(bgd, block_buffer + (index * sizeof(ext2_block_group_descriptor)), sizeof(ext2_block_group_descriptor));

    dbgprint("Block usage bitmap: %d\n", bgd->block_usage_bitmap);
    dbgprint("Inode usage bitmap: %d\n", bgd->inode_usage_bitmap);
    dbgprint("Inode table block: %d\n", bgd->inode_table_block);
    dbgprint("Free blocks: %d\n", bgd->free_blocks);
    dbgprint("Free inodes: %d\n", bgd->free_inodes);
    dbgprint("Directories: %d\n", bgd->directories);

    free(block_buffer);

    return 0;
}

static int ext2_read_inode(iodriver *driver, filesystem *fs, ext2_inode *inode, int index) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    int block_group = (index - 1) / superblock->base.inodes_per_group;
    int inode_index = (index - 1) % superblock->base.inodes_per_group;

    ext2_block_group_descriptor bgd;
    ext2_read_bgd(driver, fs, &bgd, block_group);

    int block = bgd.inode_table_block + (inode_index * sizeof(ext2_inode)) / block_size;
    int offset = (inode_index * sizeof(ext2_inode)) % block_size;
    dbgprint("inode: %d\n", index);
    dbgprint("block group: %d\n", block_group);
    dbgprint("inode index: %d\n", inode_index);
    dbgprint("block: %d\n", block);
    dbgprint("offset: %d\n", offset);

    uint8_t *block_buffer = malloc(block_size);
    if (!ext2_read_block(driver, fs, block, block_buffer)) {
        free(block_buffer);
        return -EIO;
    }

    memcpy(inode, block_buffer + offset, sizeof(ext2_inode));

    dbgprint("Inode type/permissions: %hx\n", inode->type_permission);
    dbgprint("Inode user ID: %d\n", inode->user_id);
    dbgprint("Inode size: %d\n", inode->size);
    dbgprint("Inode last access time: %d\n", inode->last_access_time);
    dbgprint("Inode creation time: %d\n", inode->creation_time);
    dbgprint("Inode last modification time: %d\n", inode->last_modification_time);
    dbgprint("Inode deletion time: %d\n", inode->deletion_time);
    dbgprint("Inode group ID: %d\n", inode->group_id);
    dbgprint("Inode hard links: %d\n", inode->hard_links);
    dbgprint("Inode disk sectors: %d\n", inode->disk_sectors);
    dbgprint("Inode flags: %d\n", inode->flags);
    dbgprint("Inode OS specific value: %d\n", inode->os_specific_value);
    dbgprint("Inode direct block pointers: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", inode->direct_block_pointers[0], inode->direct_block_pointers[1], inode->direct_block_pointers[2], inode->direct_block_pointers[3], inode->direct_block_pointers[4], inode->direct_block_pointers[5], inode->direct_block_pointers[6], inode->direct_block_pointers[7], inode->direct_block_pointers[8], inode->direct_block_pointers[9], inode->direct_block_pointers[10], inode->direct_block_pointers[11]);
    dbgprint("Inode singly indirect block pointer: %d\n", inode->singly_indirect_block_pointer);
    dbgprint("Inode doubly indirect block pointer: %d\n", inode->doubly_indirect_block_pointer);
    dbgprint("Inode triply indirect block pointer: %d\n", inode->triply_indirect_block_pointer);
    dbgprint("Inode generation number: %d\n", inode->generation_number);
    dbgprint("Inode extended attribute block: %d\n", inode->extended_attribute_block);
    dbgprint("Inode size high: %d\n", inode->size_high);
    dbgprint("Inode fragment block address: %d\n", inode->fragment_block_address);

    free(block_buffer);

    return 0;
}

static void ext2_stat_fill(iodriver *driver, filesystem *fs, ext2_inode *f, struct stat *st) {
    if (!f) {
        return;
    }

    st->st_dev = driver->device;
    st->st_ino = 0;
    st->st_mode = f->type_permission; // ext2 inode type/permissions are the same as POSIX st_mode
    st->st_nlink = f->hard_links;
    st->st_uid = f->user_id;
    st->st_gid = f->group_id;
    st->st_rdev = 0;
    st->st_size = ((!ISSET_BIT_INT(f->type_permission, EXT2_INODE_TYPE_DIRECTORY) ? (uint64_t)f->size_high : 0L) << 32) | f->size;
    st->st_blksize = 1024 << ((ext2_extended_superblock *)fs->params)->base.log2_block_size;
    st->st_blocks = f->disk_sectors;
    st->st_atime = f->last_access_time;
    st->st_ctime = f->creation_time;
    st->st_mtime = f->last_modification_time;
    st->st_private = (void *)f;
}

void ext2_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading Superblock (sector %x)...\n", fs->start_lba);

    ext2_extended_superblock *superblock = malloc(sizeof(ext2_extended_superblock));

    if (driver->sector_size == 512) {
        driver->read_sector(driver, fs->start_lba + 2, driver->io_buffer, true);
        memcpy(superblock, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);
        driver->read_sector(driver, fs->start_lba + 3, driver->io_buffer, true);
        memcpy(((uint8_t *)superblock) + sizeof(ext2_extended_superblock) / 2, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);
    } else {
        driver->read_sector(driver, fs->start_lba + 2, driver->io_buffer, true);
        memcpy(superblock, driver->io_buffer, sizeof(ext2_extended_superblock));
    }

    if (superblock->base.ext2_signature != EXT2_SIGNATURE) {
        dbgprint("Invalid signature: %x\n", superblock->base.ext2_signature);
        return;
    }

    if ((1024 << superblock->base.log2_block_size) < driver->sector_size) {
        panic("Sector size (%d) is too small for drive (%d)\n", 1024 << superblock->base.log2_block_size, driver->sector_size);
    } else if ((1024 << superblock->base.log2_block_size) % driver->sector_size) {
        panic("Sector size (%d) is not a multiple of block size (%d)\n", 1024 << superblock->base.log2_block_size, driver->sector_size);
    }

    fs->params = superblock;
    fs->case_sensitive = true;

    printf("Volume label is %s\n", superblock->volume_name);
    printf("File system is %s\n", "EXT2");
    printf("Serial number is %s\n", printuuid(superblock->filesystem_id));

    dbgprint("Inodes: %d\n", superblock->base.inodes);
    dbgprint("Blocks: %d\n", superblock->base.blocks);
    dbgprint("Reserved blocks: %d\n", superblock->base.reserved_blocks);
    dbgprint("Free blocks: %d\n", superblock->base.free_blocks);
    dbgprint("Free inodes: %d\n", superblock->base.free_inodes);

    dbgprint("Block size: %d\n", 1024 << superblock->base.log2_block_size);
    dbgprint("Fragment size: %d\n", 1024 << superblock->base.log2_fragment_size);

    dbgprint("Blocks per group: %d\n", superblock->base.blocks_per_group);
    dbgprint("Fragments per group: %d\n", superblock->base.fragments_per_group);
    dbgprint("Inodes per group: %d\n", superblock->base.inodes_per_group);

    struct stat *st = calloc(1, sizeof(struct stat));
    ext2_inode *rootdir_inode = (ext2_inode *)malloc(sizeof(ext2_inode));
    ext2_read_inode(driver, fs, rootdir_inode, 2);
    ext2_stat_fill(driver, fs, rootdir_inode, st);
    st->st_ino = 2;
    fs->rootdir = st;
}

int ext2_stat(iodriver *driver, filesystem *fs, const struct stat *st, const char *path, struct stat *out_st) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
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
            if (ext2_readdir(driver, fs, &dir, i, name, out_st) < 0) {
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

static int ext2_read_singly_indirect_block_pointer(iodriver *driver, filesystem *fs, int block_pointer, void *addr, size_t count, size_t offset) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    dbgprint("Reading %ld bytes starting from %ld of singly indirect block pointer %d\n", count, offset, block_pointer);

    uint32_t *singly_indirect_block = (uint32_t *)malloc(block_size);
    if (!ext2_read_block(driver, fs, block_pointer, (uint8_t *)singly_indirect_block)) {
        free(singly_indirect_block);
        return -EIO;
    }

    size_t bytes_read = 0;
    uint8_t *block_buffer = malloc(block_size);
    for (int i = offset / block_size / sizeof(uint32_t); i < block_size / sizeof(uint32_t) && count; i++) {
        size_t to_read = count < block_size ? count : block_size;

        uint32_t data_block = ((uint32_t *)singly_indirect_block)[i];
        if (!data_block) {
            break;
        }

        dbgprint("Reading %ld bytes starting from %ld of data block %d\n", to_read, offset, data_block);
        if (!ext2_read_block(driver, fs, data_block, block_buffer)) {
            free(block_buffer);
            free(singly_indirect_block);
            return -EIO;
        }

        size_t off = offset % block_size;
        size_t len = off < to_read ? to_read - off : to_read;

        memcpy((uint8_t *)addr + bytes_read, block_buffer + off, len);
        memcpy((uint8_t *)addr + bytes_read + len, block_buffer + off + len, to_read - len);

        bytes_read += to_read;
        count -= to_read;
    }

    free(block_buffer);
    free(singly_indirect_block);

    dbgprint("Read %ld bytes\n", bytes_read);

    return bytes_read;
}

static int serial_write_fn(const char *str, ...) {
    va_list args;
    va_start(args, str);
    serial_write_str_varargs(SERIAL_COM1, str, args);
    va_end(args);

    return 0;
}

int ext2_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    ext2_inode *f = (ext2_inode *)st->st_private;
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

    size_t bytes_read = 0;
    uint8_t *block_buffer = malloc(block_size);
    for (int i = offset / block_size; i < 12 && f->direct_block_pointers[i] && count; i++) {
        size_t to_read = count < block_size ? count : block_size;

        int data_block = f->direct_block_pointers[i];
        if (!ext2_read_block(driver, fs, data_block, block_buffer)) {
            free(block_buffer);
            return -EIO;
        }

        size_t off = offset % block_size;
        size_t len = off < to_read ? to_read - off : to_read;

        memcpy((uint8_t *)buf + bytes_read, block_buffer + off, len);
        memcpy((uint8_t *)buf + bytes_read + len, block_buffer + off + len, to_read - len);

        bytes_read += to_read;
        count -= to_read;
    }

    free(block_buffer);

    if (count && f->singly_indirect_block_pointer) {
        uint32_t singly_indirect_block_pointer = f->singly_indirect_block_pointer;

        int r = ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, (uint8_t *)buf + bytes_read, count, offset < bytes_read ? 0 : offset - bytes_read);
        if (r < 0) {
            return r;
        }

        bytes_read += r;
        count -= r;
    }

    if (count && f->doubly_indirect_block_pointer) {
        uint32_t doubly_indirect_block_pointer = f->doubly_indirect_block_pointer;

        uint32_t *doubly_indirect_block = (uint32_t *)malloc(block_size);
        if (!ext2_read_block(driver, fs, doubly_indirect_block_pointer, (uint8_t *)doubly_indirect_block)) {
            free(doubly_indirect_block);
            return -EIO;
        }

        for (int i = 0; i < block_size / sizeof(uint32_t) && count; i++) {
            uint32_t singly_indirect_block_pointer = doubly_indirect_block[i];
            if (!singly_indirect_block_pointer) {
                break;
            }

            int r = ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, (uint8_t *)buf + bytes_read, count, offset < bytes_read ? 0 : offset - bytes_read);
            if (r < 0) {
                free(doubly_indirect_block);
                return r;
            }

            bytes_read += r;
            count -= r;
        }

        free(doubly_indirect_block);
    }

    if (count && f->triply_indirect_block_pointer) {
        uint32_t triply_indirect_block_pointer = f->triply_indirect_block_pointer;

        uint32_t *triply_indirect_block = (uint32_t *)malloc(block_size);
        if (!ext2_read_block(driver, fs, triply_indirect_block_pointer, (uint8_t *)triply_indirect_block)) {
            free(triply_indirect_block);
            return -EIO;
        }

        for (int i = 0; i < block_size / sizeof(uint32_t) && count; i++) {
            uint32_t doubly_indirect_block_pointer = triply_indirect_block[i];
            if (!doubly_indirect_block_pointer) {
                break;
            }

            uint32_t *doubly_indirect_block = (uint32_t *)malloc(block_size);
            if (!ext2_read_block(driver, fs, doubly_indirect_block_pointer, (uint8_t *)doubly_indirect_block)) {
                free(doubly_indirect_block);
                free(triply_indirect_block);
                return -EIO;
            }

            for (int j = 0; j < block_size / sizeof(uint32_t) && count; j++) {
                uint32_t singly_indirect_block_pointer = doubly_indirect_block[j];
                if (!singly_indirect_block_pointer) {
                    break;
                }

                int r = ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, (uint8_t *)buf + bytes_read, count, offset < bytes_read ? 0 : offset - bytes_read);
                if (r < 0) {
                    free(doubly_indirect_block);
                    free(triply_indirect_block);
                    return r;
                }

                bytes_read += r;
                count -= r;
            }

            free(doubly_indirect_block);
        }

        free(triply_indirect_block);
    }

    return bytes_read;
}

int ext2_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset) {
    // Read-only filesystem
    return -EROFS;
}

int ext2_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    ext2_inode *f = (ext2_inode *)st->st_private;
    if (!f) {
        return -EBADFD;
    }

    if (!ISSET_BIT_INT(f->type_permission, EXT2_INODE_TYPE_DIRECTORY)) {
        return -ENOTDIR;
    }

    size_t i = 0;
    size_t j = 0;
    uint8_t *block_buffer = calloc(2, block_size);
    while (ext2_read(driver, fs, st, block_buffer, 2 * block_size, i) > 0) {
        int offset = 0;
        ext2_directory_entry entry;

        while (offset < block_size) {
            memcpy(&entry, block_buffer + offset, sizeof(ext2_directory_entry));

            if (entry.inode == 0) {
                free(block_buffer);
                return -ENOENT;
            }

            if (j == index) {
                strncpy(name, entry.name, entry.name_length);

                ext2_inode *inode = (ext2_inode *)malloc(sizeof(ext2_inode));
                if (ext2_read_inode(driver, fs, inode, entry.inode)) {
                    free(inode);
                    free(block_buffer);
                    return -EIO;
                }

                ext2_stat_fill(driver, fs, inode, out_st);
                out_st->st_ino = entry.inode;

                free(block_buffer);

                return 1;
            }

            offset += entry.size;
            j++;
        }

        i += block_size;
        memset(block_buffer, 0, 2 * block_size);
    }

    free(block_buffer);

    return -ENOENT;
}
