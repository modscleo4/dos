#include "ext2.h"

#include <stdlib.h>
#include "../../debug.h"

/**
 * How To Read An Inode
 *
 *  1. Read the Superblock to find the size of each block, the number of blocks per group, number Inodes per group, and the starting block of the first group (Block Group Descriptor Table).
 *  2. Determine which block group the inode belongs to.
 *  3. Read the Block Group Descriptor corresponding to the Block Group which contains the inode to be looked up.
 *  4. From the Block Group Descriptor, extract the location of the block group's inode table.
 *  5. Determine the index of the inode in the inode table.
 *  6. Index the inode table (taking into account non-standard inode size).
 *
 * Directory entry information and file contents are located within the data blocks that the Inode points to.
 */

/**
 * How To Read the Root Directory
 *
 * The root directory's inode is defined to always be 2. Read/parse the contents of inode 2.
 */

ext2_extended_superblock superblock;

void ext2_init(iodriver *driver, filesystem *fs) {
    dbgprint("Reading Superblock (sector %x)...\n", fs->start_lba);

    driver->read_sector(driver->device, fs->start_lba + 2, driver->io_buffer, true);
    memcpy(&superblock, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);
    driver->read_sector(driver->device, fs->start_lba + 3, driver->io_buffer, true);
    memcpy(&superblock + sizeof(ext2_extended_superblock) / 2, driver->io_buffer, sizeof(ext2_extended_superblock) / 2);

    if (superblock.base.ext2_signature != EXT2_SIGNATURE) {
        dbgprint("Invalid signature: %x\n", superblock.base.ext2_signature);
        return;
    }

    printf("Volume label is %s\n", superblock.volume_name);
    printf("File system is %s\n", "EXT2");
    printf("Serial number is %s\n", printuuid(superblock.filesystem_id));

    dbgprint("Inodes: %d\n", superblock.base.inodes);
    dbgprint("Blocks: %d\n", superblock.base.blocks);
    dbgprint("Reserved blocks: %d\n", superblock.base.reserved_blocks);
    dbgprint("Free blocks: %d\n", superblock.base.free_blocks);
    dbgprint("Free inodes: %d\n", superblock.base.free_inodes);

    dbgprint("Block size: %d\n", 1024 << superblock.base.log2_block_size);
    dbgprint("Fragment size: %d\n", 1024 << superblock.base.log2_fragment_size);

    dbgprint("Blocks per group: %d\n", superblock.base.blocks_per_group);
    dbgprint("Fragments per group: %d\n", superblock.base.fragments_per_group);
    dbgprint("Inodes per group: %d\n", superblock.base.inodes_per_group);
}

unsigned long int ext2_get_file_size(iodriver *driver, const void *_f) {
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

void ext2_list_files(iodriver *driver, filesystem *fs) {
    int block_size = 1024 << superblock.base.log2_block_size;
    int bgd_block = block_size == 1024 ? 2 : 1;

    ext2_block_group_descriptor bgd;
    driver->read_sector(driver->device, fs->start_lba + bgd_block * block_size / 512, driver->io_buffer, true);
    memcpy(&bgd, driver->io_buffer, sizeof(ext2_block_group_descriptor));

    dbgprint("Block usage bitmap: %d\n", bgd.block_usage_bitmap);
    dbgprint("Inode usage bitmap: %d\n", bgd.inode_usage_bitmap);
    dbgprint("Inode table block: %d\n", bgd.inode_table_block);
    dbgprint("Free blocks: %d\n", bgd.free_blocks);
    dbgprint("Free inodes: %d\n", bgd.free_inodes);
    dbgprint("Directories: %d\n", bgd.directories);
    dbgwait();

    int rootdir_inode = 2;
    int rootdir_block_group = (rootdir_inode - 1) / superblock.base.inodes_per_group;
    int rootdir_inode_index = (rootdir_inode - 1) % superblock.base.inodes_per_group;
    int rootdir_block = bgd.inode_table_block + (rootdir_inode_index * sizeof(ext2_inode)) / block_size;
    dbgprint("Root directory inode: %d\n", rootdir_inode);
    dbgprint("Root directory block group: %d\n", rootdir_block_group);
    dbgprint("Root directory inode index: %d\n", rootdir_inode_index);
    dbgprint("Root directory block: %d\n", rootdir_block);

    ext2_inode inode;
    driver->read_sector(driver->device, fs->start_lba + rootdir_block * block_size / 512, driver->io_buffer, true);
    memcpy(&inode, driver->io_buffer + (rootdir_inode_index * sizeof(ext2_inode)), sizeof(ext2_inode));

    dbgprint("Inode type: %hd\n", inode.type);
    dbgprint("Inode permissions: %d\n", inode.permissions);
    dbgprint("Inode user ID: %d\n", inode.user_id);
    dbgprint("Inode size: %d\n", inode.size);
    dbgprint("Inode last access time: %d\n", inode.last_access_time);
    dbgprint("Inode creation time: %d\n", inode.creation_time);
    dbgprint("Inode last modification time: %d\n", inode.last_modification_time);
    dbgprint("Inode deletion time: %d\n", inode.deletion_time);
    dbgprint("Inode group ID: %d\n", inode.group_id);
    dbgprint("Inode hard links: %d\n", inode.hard_links);
    dbgprint("Inode disk sectors: %d\n", inode.disk_sectors);
    dbgprint("Inode flags: %d\n", inode.flags);
    dbgprint("Inode OS specific value: %d\n", inode.os_specific_value);
    dbgprint("Inode direct block pointers: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", inode.direct_block_pointers[0], inode.direct_block_pointers[1], inode.direct_block_pointers[2], inode.direct_block_pointers[3], inode.direct_block_pointers[4], inode.direct_block_pointers[5], inode.direct_block_pointers[6], inode.direct_block_pointers[7], inode.direct_block_pointers[8], inode.direct_block_pointers[9], inode.direct_block_pointers[10], inode.direct_block_pointers[11]);
    dbgprint("Inode singly indirect block pointer: %d\n", inode.singly_indirect_block_pointer);
    dbgprint("Inode doubly indirect block pointer: %d\n", inode.doubly_indirect_block_pointer);
    dbgprint("Inode triply indirect block pointer: %d\n", inode.triply_indirect_block_pointer);
    dbgprint("Inode generation number: %d\n", inode.generation_number);
    dbgprint("Inode extended attribute block: %d\n", inode.extended_attribute_block);
    dbgprint("Inode size high: %d\n", inode.size_high);
    dbgprint("Inode fragment block address: %d\n", inode.fragment_block_address);
    dbgwait();

    ext2_directory_entry entry;
    int data_block = inode.direct_block_pointers[0];
    driver->read_sector(driver->device, fs->start_lba + data_block * block_size / 512, driver->io_buffer, true);
    memcpy(&entry, driver->io_buffer, sizeof(ext2_directory_entry));

    dbgprint("Directory entry inode: %d\n", entry.inode);
    dbgprint("Directory entry size: %d\n", entry.size);
    dbgprint("Directory entry name length: %d\n", entry.name_length);
    dbgprint("Directory entry type: %d\n", entry.type_indicator);
    dbgprint("Directory entry name: %s\n", entry.name);
}
