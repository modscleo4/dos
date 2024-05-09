#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>
#include "../filesystem.h"
#include "../iodriver.h"
#include "../../modules/uuid.h"

typedef struct ext2_base_superblock {
    uint32_t inodes;
    uint32_t blocks;
    uint32_t reserved_blocks;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t superblock_block;
    uint32_t log2_block_size;
    uint32_t log2_fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t mounts_since_last_check;
    uint16_t mounts_allowed_before_check;
    uint16_t ext2_signature;
    uint16_t state;
    uint16_t error_handling;
    uint16_t minor_version;
    uint32_t last_check_time;
    uint32_t check_interval;
    uint32_t os_id;
    uint32_t major_version;
    uint16_t user_id;
    uint16_t group_id;
} ext2_base_superblock;

typedef struct ext2_extended_superblock {
    ext2_base_superblock base;
    uint32_t first_non_reserved_inode;
    uint16_t inode_size;
    uint16_t superblock_block_group;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t read_only_features;
    uuid filesystem_id;
    char volume_name[16];
    char last_mounted_path[64];
    uint32_t compression_info;
    uint8_t preallocated_file_blocks;
    uint8_t preallocated_dir_blocks;
    uint16_t unused;
    uuid journal_id;
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_list;
    uint8_t unused2[788];
} ext2_extended_superblock;

typedef struct ext2_block_group_descriptor {
    uint32_t block_usage_bitmap;
    uint32_t inode_usage_bitmap;
    uint32_t inode_table_block;
    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t directories;
    uint8_t unused[14];
} ext2_block_group_descriptor;

typedef struct ext2_inode {
    uint16_t type_permission;
    uint16_t user_id;
    uint32_t size;
    uint32_t last_access_time;
    uint32_t creation_time;
    uint32_t last_modification_time;
    uint32_t deletion_time;
    uint16_t group_id;
    uint16_t hard_links;
    uint32_t disk_sectors;
    uint32_t flags;
    uint32_t os_specific_value;
    uint32_t direct_block_pointers[12];
    uint32_t singly_indirect_block_pointer;
    uint32_t doubly_indirect_block_pointer;
    uint32_t triply_indirect_block_pointer;
    uint32_t generation_number;
    uint32_t extended_attribute_block;
    uint32_t size_high; // Directory ACL or upper 32 bits of size
    uint32_t fragment_block_address;
    uint8_t os_specific[12];
} ext2_inode;

typedef struct ext2_directory_entry {
    uint32_t inode;
    uint16_t size;
    uint8_t name_length;
    uint8_t type_indicator;
    char name[256];
} ext2_directory_entry;

#define EXT2_SIGNATURE 0xEF53

enum Ext2FSState {
    EXT2_FS_CLEAN = 1,
    EXT2_FS_ERRORS,
};

enum EXT2FSErrorHandling {
    EXT2_FS_IGNORE = 1,
    EXT2_FS_REMOUNT_READ_ONLY,
    EXT2_FS_KERNEL_PANIC,
};

enum Ext2OSID {
    EXT2_OS_LINUX = 0,
    EXT2_OS_HURD,
    EXT2_OS_MASIX,
    EXT2_OS_FREEBSD,
    EXT2_OS_OTHER_LITES,
};

enum Ext2InodeType {
    EXT2_INODE_TYPE_FIFO = 0x1000,
    EXT2_INODE_TYPE_CHAR_DEVICE = 0x2000,
    EXT2_INODE_TYPE_DIRECTORY = 0x4000,
    EXT2_INODE_TYPE_BLOCK_DEVICE = 0x6000,
    EXT2_INODE_TYPE_FILE = 0x8000,
    EXT2_INODE_TYPE_SYMBOLIC_LINK = 0xA000,
    EXT2_INODE_TYPE_SOCKET = 0xC000,
};

enum Ext2InodePermissions {
    EXT2_INODE_PERMISSION_OTHER_EXECUTE = 0x0001,
    EXT2_INODE_PERMISSION_OTHER_WRITE = 0x0002,
    EXT2_INODE_PERMISSION_OTHER_READ = 0x0004,
    EXT2_INODE_PERMISSION_GROUP_EXECUTE = 0x0008,
    EXT2_INODE_PERMISSION_GROUP_WRITE = 0x0010,
    EXT2_INODE_PERMISSION_GROUP_READ = 0x0020,
    EXT2_INODE_PERMISSION_USER_EXECUTE = 0x0040,
    EXT2_INODE_PERMISSION_USER_WRITE = 0x0080,
    EXT2_INODE_PERMISSION_USER_READ = 0x0100,
    EXT2_INODE_PERMISSION_STICKY_BIT = 0x0200,
    EXT2_INODE_PERMISSION_SET_GROUP_ID = 0x0400,
    EXT2_INODE_PERMISSION_SET_USER_ID = 0x0800,
};

enum Ext2InodeFlags {
    EXT2_INODE_FLAG_SECURE_DELETION = 0x0001,
    EXT2_INODE_FLAG_KEEP_ON_DISK_AFTER_DELETION = 0x0002,
    EXT2_INODE_FLAG_COMPRESSED_FILE = 0x0004,
    EXT2_INODE_FLAG_SYNCHRONOUS_UPDATES = 0x0008,
    EXT2_INODE_FLAG_IMMUTABLE = 0x0010,
    EXT2_INODE_FLAG_APPEND_ONLY = 0x0020,
    EXT2_INODE_FLAG_NO_DUMP = 0x0040,
    EXT2_INODE_FLAG_NO_LAST_ACCESS_TIME_UPDATE = 0x0080,
    EXT2_INODE_FLAG_HASH_INDEXED_DIRECTORY = 0x10000,
    EXT2_INODE_FLAG_AFS_DIRECTORY = 0x20000,
    EXT2_INODE_FLAG_JOURNAL_FILE_DATA = 0x40000,
};

enum Ext2DirectoryEntryType {
    EXT2_DIRECTORY_ENTRY_TYPE_UNKNOWN = 0,
    EXT2_DIRECTORY_ENTRY_TYPE_REGULAR_FILE,
    EXT2_DIRECTORY_ENTRY_TYPE_DIRECTORY,
    EXT2_DIRECTORY_ENTRY_TYPE_CHARACTER_DEVICE,
    EXT2_DIRECTORY_ENTRY_TYPE_BLOCK_DEVICE,
    EXT2_DIRECTORY_ENTRY_TYPE_FIFO,
    EXT2_DIRECTORY_ENTRY_TYPE_SOCKET,
    EXT2_DIRECTORY_ENTRY_TYPE_SYMBOLIC_LINK,
};

void ext2_init(iodriver *driver, filesystem *fs);

int ext2_stat(iodriver *driver, filesystem *fs, const char *path, struct stat *st);

void *ext2_load_file(iodriver *driver, filesystem *fs, const struct stat *st);

int ext2_read(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int ext2_write(iodriver *driver, filesystem *fs, const struct stat *st, void *buf, size_t count, size_t offset);

int ext2_readdir(iodriver *driver, filesystem *fs, const struct stat *st, size_t index, char *name, struct stat *out_st);

void ext2_list_files(iodriver *driver, filesystem *fs);

#endif // EXT2_H
