#include "ext2.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/panic.h"
#include "../../modules/bitmap.h"

static uint8_t *block_buffer;

static void ext2_read_block(iodriver *driver, filesystem *fs, int block) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    for (int i = 0; i < block_size; i += driver->sector_size) {
        driver->read_sector(driver, fs->start_lba + block * block_size / driver->sector_size + i / driver->sector_size, block_buffer + i, true);
    }
}

static void ext2_read_bgd(iodriver *driver, filesystem *fs, ext2_block_group_descriptor *bgd, int index) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    ext2_read_block(driver, fs, bgd_block);
    memcpy(bgd, block_buffer + (index * sizeof(ext2_block_group_descriptor)), sizeof(ext2_block_group_descriptor));

    // dbgprint("Block usage bitmap: %d\n", bgd->block_usage_bitmap);
    // dbgprint("Inode usage bitmap: %d\n", bgd->inode_usage_bitmap);
    // dbgprint("Inode table block: %d\n", bgd->inode_table_block);
    // dbgprint("Free blocks: %d\n", bgd->free_blocks);
    // dbgprint("Free inodes: %d\n", bgd->free_inodes);
    // dbgprint("Directories: %d\n", bgd->directories);
    // dbgwait();
}

static void ext2_read_inode(iodriver *driver, filesystem *fs, ext2_inode *inode, int index) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    int block_group = (index - 1) / superblock->base.inodes_per_group;
    int inode_index = (index - 1) % superblock->base.inodes_per_group;

    ext2_block_group_descriptor bgd;
    ext2_read_bgd(driver, fs, &bgd, block_group);

    int block = bgd.inode_table_block + (inode_index * sizeof(ext2_inode)) / block_size;
    // dbgprint("inode: %d\n", index);
    // dbgprint("block group: %d\n", block_group);
    // dbgprint("inode index: %d\n", inode_index);
    // dbgprint("block: %d\n", block);

    ext2_read_block(driver, fs, block);
    memcpy(inode, block_buffer + (inode_index * sizeof(ext2_inode)), sizeof(ext2_inode));

    // dbgprint("Inode type: %hd\n", inode->type);
    // dbgprint("Inode permissions: %d\n", inode->permissions);
    // dbgprint("Inode user ID: %d\n", inode->user_id);
    // dbgprint("Inode size: %d\n", inode->size);
    // dbgprint("Inode last access time: %d\n", inode->last_access_time);
    // dbgprint("Inode creation time: %d\n", inode->creation_time);
    // dbgprint("Inode last modification time: %d\n", inode->last_modification_time);
    // dbgprint("Inode deletion time: %d\n", inode->deletion_time);
    // dbgprint("Inode group ID: %d\n", inode->group_id);
    // dbgprint("Inode hard links: %d\n", inode->hard_links);
    // dbgprint("Inode disk sectors: %d\n", inode->disk_sectors);
    // dbgprint("Inode flags: %d\n", inode->flags);
    // dbgprint("Inode OS specific value: %d\n", inode->os_specific_value);
    // dbgprint("Inode direct block pointers: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", inode->direct_block_pointers[0], inode->direct_block_pointers[1], inode->direct_block_pointers[2], inode->direct_block_pointers[3], inode->direct_block_pointers[4], inode->direct_block_pointers[5], inode->direct_block_pointers[6], inode->direct_block_pointers[7], inode->direct_block_pointers[8], inode->direct_block_pointers[9], inode->direct_block_pointers[10], inode->direct_block_pointers[11]);
    // dbgprint("Inode singly indirect block pointer: %d\n", inode->singly_indirect_block_pointer);
    // dbgprint("Inode doubly indirect block pointer: %d\n", inode->doubly_indirect_block_pointer);
    // dbgprint("Inode triply indirect block pointer: %d\n", inode->triply_indirect_block_pointer);
    // dbgprint("Inode generation number: %d\n", inode->generation_number);
    // dbgprint("Inode extended attribute block: %d\n", inode->extended_attribute_block);
    // dbgprint("Inode size high: %d\n", inode->size_high);
    // dbgprint("Inode fragment block address: %d\n", inode->fragment_block_address);
    // dbgwait();
}

void ext2_init(iodriver *driver, filesystem *fs) {
    block_buffer = malloc_align(2048, BITMAP_PAGE_SIZE);

    dbgprint("Reading Superblock (sector %x)...\n", fs->start_lba);

    ext2_extended_superblock *superblock = malloc(sizeof(ext2_extended_superblock));

    driver->read_sector(driver, fs->start_lba + 2, block_buffer, true);
    memcpy(superblock, block_buffer, sizeof(ext2_extended_superblock) / 2);
    driver->read_sector(driver, fs->start_lba + 3, block_buffer, true);
    memcpy(((uint8_t *)superblock) + sizeof(ext2_extended_superblock) / 2, block_buffer, sizeof(ext2_extended_superblock) / 2);

    if (superblock->base.ext2_signature != EXT2_SIGNATURE) {
        dbgprint("Invalid signature: %x\n", superblock->base.ext2_signature);
        return;
    }

    fs->params = superblock;

    printf("Volume label is %s\n", superblock->volume_name);
    printf("File system is %s\n", "EXT2");
    printf("Serial number is %s\n", printuuid(superblock->filesystem_id));

    //dbgprint("Inodes: %d\n", superblock->base.inodes);
    //dbgprint("Blocks: %d\n", superblock->base.blocks);
    //dbgprint("Reserved blocks: %d\n", superblock->base.reserved_blocks);
    //dbgprint("Free blocks: %d\n", superblock->base.free_blocks);
    //dbgprint("Free inodes: %d\n", superblock->base.free_inodes);

    //dbgprint("Block size: %d\n", 1024 << superblock->base.log2_block_size);
    //dbgprint("Fragment size: %d\n", 1024 << superblock->base.log2_fragment_size);

    //dbgprint("Blocks per group: %d\n", superblock->base.blocks_per_group);
    //dbgprint("Fragments per group: %d\n", superblock->base.fragments_per_group);
    //dbgprint("Inodes per group: %d\n", superblock->base.inodes_per_group);
}

size_t ext2_get_file_size(filesystem *fs, const void *_f) {
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return 0;
    }

    return f->size;
}

ext2_inode *ext2_search_file(iodriver *driver, filesystem *fs, const char *filename) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    int rootdir_inode_index = 2;

    ext2_inode rootdir_inode;
    ext2_read_inode(driver, fs, &rootdir_inode, rootdir_inode_index);

    for (int i = 0; i < 12 && rootdir_inode.direct_block_pointers[i]; i++) {
        int data_block = rootdir_inode.direct_block_pointers[0];
        int offset = 0;

        while (offset < block_size) {
            ext2_directory_entry entry;
            static ext2_inode f;
            ext2_read_block(driver, fs, data_block);
            memcpy(&entry, &block_buffer[offset], sizeof(ext2_directory_entry));

            char name[256];
            char cmpname[256];
            memcpy(name, entry.name, entry.name_length);
            name[entry.name_length] = 0;
            memcpy(cmpname, filename, strlen(filename));
            cmpname[strlen(filename)] = 0;

            if (!strcmp(strupr(name), strupr(cmpname))) {
                ext2_read_inode(driver, fs, &f, entry.inode);
                if (ISSET_BIT_INT(f.type_permission, EXT2_INODE_TYPE_DIRECTORY)) {
                    return NULL;
                }

                return &f;
            }

            offset += entry.size;
        }
    }

    return NULL;
}

void *ext2_load_file(iodriver *driver, filesystem *fs, const void *_f) {
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return 0;
    }

    void *addr = malloc_align(f->size, BITMAP_PAGE_SIZE);
    return ext2_load_file_at(driver, fs, f, addr);
}

static size_t ext2_read_singly_indirect_block_pointer(iodriver *driver, filesystem *fs, int block_pointer, void *addr) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    size_t offset = 0;

    for (int i = 0; i < block_size / sizeof(uint32_t); i++) {
        ext2_read_block(driver, fs, block_pointer);

        uint32_t data_block = ((uint32_t *)block_buffer)[i];
        if (!data_block) {
            break;
        }

        ext2_read_block(driver, fs, data_block);

        memcpy(addr + offset, block_buffer, block_size);
        offset += block_size;
    }

    return offset;
}

void *ext2_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return NULL;
    }

    size_t offset = 0;
    for (int i = 0; i < 12 && f->direct_block_pointers[i]; i++) {
        int data_block = f->direct_block_pointers[i];
        ext2_read_block(driver, fs, data_block);

        memcpy(addr + offset, block_buffer, block_size);
        offset += block_size;
    }

    if (f->singly_indirect_block_pointer) {
        int singly_indirect_block_pointer = f->singly_indirect_block_pointer;

        offset += ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, addr + offset);
    }

    if (f->doubly_indirect_block_pointer) {
        int doubly_indirect_block_pointer = f->doubly_indirect_block_pointer;

        for (int i = 0; i < block_size / sizeof(uint32_t); i++) {
            ext2_read_block(driver, fs, doubly_indirect_block_pointer);
            uint32_t singly_indirect_block_pointer = ((uint32_t *)block_buffer)[i];
            if (!singly_indirect_block_pointer) {
                break;
            }

            offset += ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, addr + offset);
        }
    }

    if (f->triply_indirect_block_pointer) {
        int triply_indirect_block_pointer = f->triply_indirect_block_pointer;

        for (int i = 0; i < block_size / sizeof(uint32_t); i++) {
            ext2_read_block(driver, fs, triply_indirect_block_pointer);
            uint32_t doubly_indirect_block_pointer = ((uint32_t *)block_buffer)[i];
            if (!doubly_indirect_block_pointer) {
                break;
            }

            for (int j = 0; j < block_size / sizeof(uint32_t); j++) {
                ext2_read_block(driver, fs, doubly_indirect_block_pointer);
                uint32_t singly_indirect_block_pointer = ((uint32_t *)block_buffer)[j];
                if (!singly_indirect_block_pointer) {
                    break;
                }

                offset += ext2_read_singly_indirect_block_pointer(driver, fs, singly_indirect_block_pointer, addr + offset);
            }
        }
    }

    return addr;
}

static void ext2_describe_file(iodriver *driver, filesystem *fs, ext2_directory_entry *d, ext2_inode *f, int level) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;

    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    printf("%x ", f->type_permission);

    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_TYPE_DIRECTORY) ? "d" : "-");

    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_USER_READ) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_USER_WRITE) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_USER_EXECUTE) ? "x" : "-");

    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_GROUP_READ) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_GROUP_WRITE) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_GROUP_EXECUTE) ? "x" : "-");

    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_OTHER_READ) ? "r" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_OTHER_WRITE) ? "w" : "-");
    printf("%s", ISSET_BIT_INT(f->type_permission, EXT2_INODE_PERMISSION_OTHER_EXECUTE) ? "x" : "-");

    printf(" 1 %04d\t%04d", f->user_id, f->group_id);

    printf("\t%ld", f->size);

    printf("\t%ld", f->creation_time);

    printf("\t%s", d->name);

    //printf(" &%X", d->inode);

    printf("\n");
}

static void ext2_list_files_in_dir(iodriver *driver, filesystem *fs, ext2_inode *inode, int level) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int j = 0;
    for (int i = 0; i < 12 && inode->direct_block_pointers[i]; i++) {
        int data_block = inode->direct_block_pointers[0];
        int offset = 0;

        while (offset < block_size) {
            ext2_directory_entry entry;
            ext2_inode f;
            ext2_read_block(driver, fs, data_block);
            memcpy(&entry, &block_buffer[offset], sizeof(ext2_directory_entry));

            if (strcmp(entry.name, ".") && strcmp(entry.name, "..")) {
                entry.name[entry.name_length] = 0;

                ext2_read_inode(driver, fs, &f, entry.inode);

                ext2_describe_file(driver, fs, &entry, &f, level);

                if (ISSET_BIT_INT(f.type_permission, EXT2_INODE_TYPE_DIRECTORY)) {
                    ext2_list_files_in_dir(driver, fs, &f, level + 1);
                }

                //dbgprint("Directory entry inode: %d\n", entry.inode);
                //dbgprint("Directory entry size: %d\n", entry.size);
                //dbgprint("Directory entry name length: %d\n", entry.name_length);
                //dbgprint("Directory entry type: %d\n", entry.type_indicator);
                //dbgprint("Directory entry name: %s\n", entry.name);
                //hexdump(&f, sizeof(ext2_inode));
                //dbgwait();
            }

            offset += entry.size;
            j++;
        }
    }
}

void ext2_list_files(iodriver *driver, filesystem *fs) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    int rootdir_inode_index = 2;

    ext2_inode rootdir_inode;
    ext2_read_inode(driver, fs, &rootdir_inode, rootdir_inode_index);

    ext2_list_files_in_dir(driver, fs, &rootdir_inode, 0);
}
