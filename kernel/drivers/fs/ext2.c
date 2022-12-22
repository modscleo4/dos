#include "ext2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../debug.h"

void ext2_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading Superblock (sector %x)...\n", fs->start_lba);

    ext2_extended_superblock *superblock = malloc(sizeof(ext2_extended_superblock));

    driver->read_sector(driver, fs->start_lba + 2, driver->io_buffer, true);
    memcpy(superblock, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);
    driver->read_sector(driver, fs->start_lba + 3, driver->io_buffer, true);
    memcpy(superblock + sizeof(ext2_extended_superblock) / 2, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);

    if (superblock->base.ext2_signature != EXT2_SIGNATURE) {
        dbgprint("Invalid signature: %x\n", superblock->base.ext2_signature);
        return;
    }

    fs->params = superblock;

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
}

size_t ext2_get_file_size(filesystem *fs, const void *_f) {
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return 0;
    }

    return f->size;
}

ext2_inode *ext2_search_file(iodriver *driver, filesystem *fs, const char *filename) {
    return NULL;
}

void *ext2_load_file(iodriver *driver, filesystem *fs, const void *_f) {
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return 0;
    }

    void *addr = malloc(f->size);
    return ext2_load_file_at(driver, fs, f, addr);
}

void *ext2_load_file_at(iodriver *driver, filesystem *fs, const void *_f, void *addr) {
    ext2_inode *f = (ext2_inode *)_f;
    if (!f) {
        return 0;
    }

    return NULL;
}

void ext2_read_block(int block, iodriver *driver, filesystem *fs) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    driver->read_sector(driver, fs->start_lba + block * block_size / driver->sector_size, driver->io_buffer, true);
}

void ext2_read_bgd(ext2_block_group_descriptor *bgd, iodriver *driver, filesystem *fs) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    ext2_read_block(bgd_block, driver, fs);
    memcpy(bgd, driver->io_buffer, sizeof(ext2_block_group_descriptor));

    dbgprint("Block usage bitmap: %d\n", bgd->block_usage_bitmap);
    dbgprint("Inode usage bitmap: %d\n", bgd->inode_usage_bitmap);
    dbgprint("Inode table block: %d\n", bgd->inode_table_block);
    dbgprint("Free blocks: %d\n", bgd->free_blocks);
    dbgprint("Free inodes: %d\n", bgd->free_inodes);
    dbgprint("Directories: %d\n", bgd->directories);
    dbgwait();
}

void ext2_read_inode(ext2_inode *inode, int index, ext2_block_group_descriptor *bgd, iodriver *driver, filesystem *fs) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;

    int block_group = (index - 1) / superblock->base.inodes_per_group;
    int inode_index = (index - 1) % superblock->base.inodes_per_group;
    int block = bgd->inode_table_block + (inode_index * sizeof(ext2_inode)) / block_size;
    dbgprint("inode: %d\n", index);
    dbgprint("block group: %d\n", block_group);
    dbgprint("inode index: %d\n", inode_index);
    dbgprint("block: %d\n", block);

    ext2_read_block(block, driver, fs);
    memcpy(inode, driver->io_buffer + (inode_index * sizeof(ext2_inode)), sizeof(ext2_inode));

    dbgprint("Inode type: %hd\n", inode->type);
    dbgprint("Inode permissions: %d\n", inode->permissions);
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
    dbgwait();
}

void ext2_read_directory_entry(ext2_directory_entry *entry, ext2_inode *inode, iodriver *driver, filesystem *fs) {
    int data_block = inode->direct_block_pointers[0];
    ext2_read_block(data_block, driver, fs);
    memcpy(entry, driver->io_buffer, sizeof(ext2_directory_entry));

    dbgprint("Directory entry inode: %d\n", entry->inode);
    dbgprint("Directory entry size: %d\n", entry->size);
    dbgprint("Directory entry name length: %d\n", entry->name_length);
    dbgprint("Directory entry type: %d\n", entry->type_indicator);
    dbgprint("Directory entry name: %s\n", entry->name);
}

void ext2_list_files(iodriver *driver, filesystem *fs) {
    ext2_extended_superblock *superblock = (ext2_extended_superblock *)fs->params;
    int block_size = 1024 << superblock->base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    ext2_block_group_descriptor bgd;
    ext2_read_bgd(&bgd, driver, fs);

    int rootdir_inode = 2;

    ext2_inode inode;
    ext2_read_inode(&inode, rootdir_inode, &bgd, driver, fs);

    ext2_directory_entry entry;
    ext2_read_directory_entry(&entry, &inode, driver, fs);
}
