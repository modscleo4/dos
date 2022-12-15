#ifndef EXT2_H
#define EXT2_H

#include "../../modules/uuid.h"
#include "../filesystem.h"
#include "../iodriver.h"

typedef struct ext2_base_superblock {
    unsigned int inodes;
    unsigned int blocks;
    unsigned int reserved_blocks;
    unsigned int free_blocks;
    unsigned int free_inodes;
    unsigned int superblock_block;
    unsigned int log2_block_size;
    unsigned int log2_fragment_size;
    unsigned int blocks_per_group;
    unsigned int fragments_per_group;
    unsigned int inodes_per_group;
    unsigned int last_mount_time;
    unsigned int last_write_time;
    unsigned short int mounts_since_last_check;
    unsigned short int mounts_allowed_before_check;
    unsigned short int ext2_signature;
    unsigned short int state;
    unsigned short int error_handling;
    unsigned short int minor_version;
    unsigned int last_check_time;
    unsigned int check_interval;
    unsigned int os_id;
    unsigned int major_version;
    unsigned short int user_id;
    unsigned short int group_id;
} ext2_base_superblock;

typedef struct ext2_extended_superblock {
    ext2_base_superblock base;
    unsigned int first_non_reserved_inode;
    unsigned short int inode_size;
    unsigned short int superblock_block_group;
    unsigned int optional_features;
    unsigned int required_features;
    unsigned int read_only_features;
    uuid filesystem_id;
    unsigned char volume_name[16];
    unsigned char last_mounted_path[64];
    unsigned int compression_info;
    unsigned char preallocated_file_blocks;
    unsigned char preallocated_dir_blocks;
    unsigned short int unused;
    unsigned char journal_id[16];
    unsigned int journal_inode;
    unsigned int journal_device;
    unsigned int orphan_inode_list;
    unsigned char unused2[788];
} __attribute__((packed)) ext2_extended_superblock;

typedef struct ext2_block_group_descriptor {
    unsigned int block_usage_bitmap;
    unsigned int inode_usage_bitmap;
    unsigned int inode_table_block;
    unsigned short int free_blocks;
    unsigned short int free_inodes;
    unsigned short int directories;
    unsigned char unused[14];
} ext2_block_group_descriptor;

typedef struct ext2_inode {
    unsigned short int type : 4;
    unsigned int permissions : 12;
    unsigned short int user_id;
    unsigned int size;
    unsigned int last_access_time;
    unsigned int creation_time;
    unsigned int last_modification_time;
    unsigned int deletion_time;
    unsigned short int group_id;
    unsigned short int hard_links;
    unsigned int disk_sectors;
    unsigned int flags;
    unsigned int os_specific_value;
    unsigned int direct_block_pointers[12];
    unsigned int singly_indirect_block_pointer;
    unsigned int doubly_indirect_block_pointer;
    unsigned int triply_indirect_block_pointer;
    unsigned int generation_number;
    unsigned int extended_attribute_block;
    unsigned int size_high; // Directory ACL or upper 32 bits of size
    unsigned int fragment_block_address;
    unsigned char os_specific[12];
} __attribute__((packed)) ext2_inode;

typedef struct ext2_directory_entry {
    unsigned int inode;
    unsigned short int size;
    unsigned char name_length;
    unsigned char type_indicator;
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
    EXT2_INODE_FIFO = 0x1000,
    EXT2_INODE_CHAR_DEVICE = 0x2000,
    EXT2_INODE_DIRECTORY = 0x4000,
    EXT2_INODE_BLOCK_DEVICE = 0x6000,
    EXT2_INODE_FILE = 0x8000,
    EXT2_INODE_SYMBOLIC_LINK = 0xA000,
    EXT2_INODE_SOCKET = 0xC000,
};

enum Ext2InodePermissions {
    EXT2_INODE_OTHER_EXECUTE = 0x0001,
    EXT2_INODE_OTHER_WRITE = 0x0002,
    EXT2_INODE_OTHER_READ = 0x0004,
    EXT2_INODE_GROUP_EXECUTE = 0x0008,
    EXT2_INODE_GROUP_WRITE = 0x0010,
    EXT2_INODE_GROUP_READ = 0x0020,
    EXT2_INODE_USER_EXECUTE = 0x0040,
    EXT2_INODE_USER_WRITE = 0x0080,
    EXT2_INODE_USER_READ = 0x0100,
    EXT2_INODE_STICKY_BIT = 0x0200,
    EXT2_INODE_SET_GROUP_ID = 0x0400,
    EXT2_INODE_SET_USER_ID = 0x0800,
};

enum Ext2InodeFlags {
    EXT2_INODE_SECURE_DELETION = 0x0001,
    EXT2_INODE_KEEP_ON_DISK_AFTER_DELETION = 0x0002,
    EXT2_INODE_COMPRESSED_FILE = 0x0004,
    EXT2_INODE_SYNCHRONOUS_UPDATES = 0x0008,
    EXT2_INODE_IMMUTABLE = 0x0010,
    EXT2_INODE_APPEND_ONLY = 0x0020,
    EXT2_INODE_NO_DUMP = 0x0040,
    EXT2_INODE_NO_LAST_ACCESS_TIME_UPDATE = 0x0080,
    EXT2_INODE_HASH_INDEXED_DIRECTORY = 0x10000,
    EXT2_INODE_AFS_DIRECTORY = 0x20000,
    EXT2_INODE_JOURNAL_FILE_DATA = 0x40000,
};

enum Ext2DirectoryEntryType {
    EXT2_DIRECTORY_ENTRY_UNKNOWN = 0,
    EXT2_DIRECTORY_ENTRY_REGULAR_FILE,
    EXT2_DIRECTORY_ENTRY_DIRECTORY,
    EXT2_DIRECTORY_ENTRY_CHARACTER_DEVICE,
    EXT2_DIRECTORY_ENTRY_BLOCK_DEVICE,
    EXT2_DIRECTORY_ENTRY_FIFO,
    EXT2_DIRECTORY_ENTRY_SOCKET,
    EXT2_DIRECTORY_ENTRY_SYMBOLIC_LINK,
};

void ext2_init(iodriver *, filesystem *);

unsigned long int ext2_get_file_size(iodriver *, const void *);

ext2_inode *ext2_search_file(iodriver *, filesystem *, const char *);

void *ext2_load_file(iodriver *, filesystem *, const void *);

void *ext2_load_file_at(iodriver *, filesystem *, const void *, void *);

void ext2_list_files(iodriver *, filesystem *);

#endif // EXT2_H
