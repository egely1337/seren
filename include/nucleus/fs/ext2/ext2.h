// SPDX-License-Identifier: Apache-2.0

#ifndef _EXT2_H
#define _EXT2_H
#include <nucleus/types.h>

#define EXT2_SIGNATURE 0xEF53

#define SECTOR_SIZE 512
#define BLOCK_SIZE  1024

struct device;
typedef enum {
	INODE_TYPE_FIFO = 0x1000,
	INODE_TYPE_CHAR_DEV = 0x2000,
	INODE_TYPE_DIRECTORY = 0x4000,
	INODE_TYPE_BLOCK_DEV = 0x6000,
	INODE_TYPE_FILE = 0x8000,
	INODE_TYPE_SYMLINK = 0xA000,
	INODE_TYPE_SOCKET = 0xC000
} inode_type;

struct ext2_sb {
	u32 inodes;
	u32 blocks;
	u32 reserved_for_root;
	u32 unallocatedblocks;
	u32 unallocatedinodes;
	u32 superblock_id;
	u32 blocksize_hint;    // shift by 1024 to the left
	u32 fragmentsize_hint; // shift by 1024 to left
	u32 blocks_in_blockgroup;
	u32 frags_in_blockgroup;
	u32 inodes_in_blockgroup;
	u32 last_mount;
	u32 last_write;
	u16 mounts_since_last_check;
	u16 max_mounts_since_last_check;
	u16 ext2_sig; // 0xEF53
	u16 state;
	u16 op_on_err;
	u16 minor_version;
	u32 last_check;
	u32 max_time_in_checks;
	u32 os_id;
	u32 major_version;
	u16 uuid;
	u16 gid;
	u8 unused[940];
} __attribute__((packed));

struct block_group_descriptor {
	u32 block_of_block_usage_bitmap;
	u32 block_of_inode_usage_bitmap;
	u32 block_of_inode_table;
	u16 num_of_unalloc_block;
	u16 num_of_unalloc_inode;
	u16 num_of_dirs;
	u8 unused[14];
} __attribute__((packed));

struct inode {
	u16 type;
	u16 uid;
	u32 size;
	u32 last_access;
	u32 create_time;
	u32 last_modif;
	u32 delete_time;
	u16 gid;
	u16 hardlinks;
	u32 disk_sectors;
	u32 flags;
	u32 ossv1;
	u32 direct_block_pointer[12];
	u32 singly_block;
	u32 doubly_block;
	u32 triply_block;
	u32 gen_no;
	u32 reserved1;
	u32 reserved2;
	u32 fragment_block;
	u8 ossv2[12];
} __attribute__((packed));

struct ext2_dir_entry {
	u32 inode;
	u16 size;
	u8 namelength;
	u8 reserved;
} __attribute__((packed));

struct ext2_priv_data {
	struct ext2_sb sb;
	u32 first_bgd;
	u32 number_of_bgs;
	u32 blocksize;
	u32 sectors_per_block;
	u32 inodes_per_block;
} __attribute__((packed));

/**
 * @brief Returns a filesystem object to mount.
 */
struct filesystem *ext2_init(struct device *dev);

/**
 * @brief Read files.
 */
void ext2_read(char *fn, const void *buf);

#endif