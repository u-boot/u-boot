/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Data structures and headers for ext4 support have been taken from
 * ext2 ls load support in Uboot
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * based on code from grub2 fs/ext2.c and fs/fshelp.c by
 * GRUB  --  GRand Unified Bootloader
 * Copyright (C) 2003, 2004  Free Software Foundation, Inc.
 */

#ifndef __EXT_COMMON__
#define __EXT_COMMON__

#include <compiler.h>

struct cmd_tbl;

#define SECTOR_SIZE		0x200
#define LOG2_SECTOR_SIZE	9

/* Magic value used to identify an ext2 filesystem.  */
#define	EXT2_MAGIC			0xEF53
/* Amount of indirect blocks in an inode.  */
#define INDIRECT_BLOCKS			12
/* Maximum lenght of a pathname.  */
#define EXT2_PATH_MAX				4096
/* Maximum nesting of symlinks, used to prevent a loop.  */
#define	EXT2_MAX_SYMLINKCNT		8
/* Maximum file name length */
#define EXT2_NAME_LEN 255

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV	0	/* The good old (original) format */
#define EXT2_DYNAMIC_REV	1	/* V2 format w/ dynamic inode sizes */

#define EXT2_GOOD_OLD_INODE_SIZE 128

/* Filetype used in directory entry.  */
#define	FILETYPE_UNKNOWN		0
#define	FILETYPE_REG			1
#define	FILETYPE_DIRECTORY		2
#define	FILETYPE_SYMLINK		7

/* Filetype information as used in inodes.  */
#define FILETYPE_INO_MASK		0170000
#define FILETYPE_INO_REG		0100000
#define FILETYPE_INO_DIRECTORY		0040000
#define FILETYPE_INO_SYMLINK		0120000
#define EXT2_ROOT_INO			2 /* Root inode */
#define EXT2_BOOT_LOADER_INO		5 /* Boot loader inode */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11

/* The size of an ext2 block in bytes.  */
#define EXT2_BLOCK_SIZE(data)	   (1 << LOG2_BLOCK_SIZE(data))

/* Log2 size of ext2 block in bytes.  */
#define LOG2_BLOCK_SIZE(data)	   (le32_to_cpu		   \
				    (data->sblock.log2_block_size) \
				    + EXT2_MIN_BLOCK_LOG_SIZE)

#define EXT2_FT_DIR	2
#define SUCCESS	1

/* Macro-instructions used to manage several block sizes  */
#define EXT2_MIN_BLOCK_LOG_SIZE	10 /* 1024 */
#define EXT2_MAX_BLOCK_LOG_SIZE	16 /* 65536 */
#define EXT2_MIN_BLOCK_SIZE		(1 << EXT2_MIN_BLOCK_LOG_SIZE)
#define EXT2_MAX_BLOCK_SIZE		(1 << EXT2_MAX_BLOCK_LOG_SIZE)

/* The ext2 superblock.  */
struct ext2_sblock {
	__le32 total_inodes;
	__le32 total_blocks;
	__le32 reserved_blocks;
	__le32 free_blocks;
	__le32 free_inodes;
	__le32 first_data_block;
	__le32 log2_block_size;
	__le32 log2_fragment_size;
	__le32 blocks_per_group;
	__le32 fragments_per_group;
	__le32 inodes_per_group;
	__le32 mtime;
	__le32 utime;
	__le16 mnt_count;
	__le16 max_mnt_count;
	__le16 magic;
	__le16 fs_state;
	__le16 error_handling;
	__le16 minor_revision_level;
	__le32 lastcheck;
	__le32 checkinterval;
	__le32 creator_os;
	__le32 revision_level;
	__le16 uid_reserved;
	__le16 gid_reserved;
	__le32 first_inode;
	__le16 inode_size;
	__le16 block_group_number;
	__le32 feature_compatibility;
	__le32 feature_incompat;
	__le32 feature_ro_compat;
	__le32 unique_id[4];
	char volume_name[16];
	char last_mounted_on[64];
	__le32 compression_info;
	uint8_t prealloc_blocks;
	uint8_t prealloc_dir_blocks;
	__le16 reserved_gdt_blocks;
	uint8_t journal_uuid[16];
	__le32 journal_inode;
	__le32 journal_dev;
	__le32 last_orphan;
	__le32 hash_seed[4];
	uint8_t default_hash_version;
	uint8_t journal_backup_type;
	__le16 descriptor_size;
	__le32 default_mount_options;
	__le32 first_meta_block_group;
	__le32 mkfs_time;
	__le32 journal_blocks[17];
	__le32 total_blocks_high;
	__le32 reserved_blocks_high;
	__le32 free_blocks_high;
	__le16 min_extra_inode_size;
	__le16 want_extra_inode_size;
	__le32 flags;
	__le16 raid_stride;
	__le16 mmp_interval;
	__le64 mmp_block;
	__le32 raid_stripe_width;
	uint8_t log2_groups_per_flex;
	uint8_t checksum_type;
};

struct ext2_block_group {
	__le32 block_id;	/* Blocks bitmap block */
	__le32 inode_id;	/* Inodes bitmap block */
	__le32 inode_table_id;	/* Inodes table block */
	__le16 free_blocks;	/* Free blocks count */
	__le16 free_inodes;	/* Free inodes count */
	__le16 used_dir_cnt;	/* Directories count */
	__le16 bg_flags;
	__le32 bg_exclude_bitmap;
	__le16 bg_block_id_csum;
	__le16 bg_inode_id_csum;
	__le16 bg_itable_unused; /* Unused inodes count */
	__le16 bg_checksum;	/* crc16(s_uuid+group_num+group_desc)*/
	/* following fields only exist if descriptor size is 64 */
	__le32 block_id_high;
	__le32 inode_id_high;
	__le32 inode_table_id_high;
	__le16 free_blocks_high;
	__le16 free_inodes_high;
	__le16 used_dir_cnt_high;
	__le16 bg_itable_unused_high;
	__le32 bg_exclude_bitmap_high;
	__le16 bg_block_id_csum_high;
	__le16 bg_inode_id_csum_high;
	__le32 bg_reserved;
};

/**
 * struct ext2_inode - ext2 inode
 *
 * For details see Linux file
 * Documentation/filesystems/ext4/inodes.rst.
 */
struct ext2_inode {
	/** @mode: file mode */
	__le16 mode;
	/** @uid: lower 16 bits of owner UID */
	__le16 uid;
	/** @size: lower 32 bits of file size */
	__le32 size;
	/** @atime: last access time */
	__le32 atime;
	/** @ctime: last change time */
	__le32 ctime;
	/** @mtime: last modification time */
	__le32 mtime;
	/** @dtime: deletion time */
	__le32 dtime;
	/** @gid: lower 16 bits of group ID */
	__le16 gid;
	/** @nlinks: number of hard links */
	__le16 nlinks;
	/** @blockcnt: lower 32 bit of block count */
	__le32 blockcnt;
	/** @flags: inode flags */
	__le32 flags;
	/** @osd1: operating system specific data */
	__le32 osd1;
	/** @b: block map or extent tree */
	union {
		struct datablocks {
			__le32 dir_blocks[INDIRECT_BLOCKS];
			__le32 indir_block;
			__le32 double_indir_block;
			__le32 triple_indir_block;
		} blocks;
		char symlink[60];
		char inline_data[60];
	} b;
	/** @version: file version (for NFS) */
	__le32 version;
	/** @acl: lower 32 bit of extended attribute block */
	__le32 acl;
	/** @size_high - dir_acl on ext2/3, upper 32 size bits on ext4
	 *
	 * In ext2/3 this field was named i_dir_acl, though it was usually set
	 * to zero and never used.
	 */
	__le32 size_high;
	/** @fragment_addr - (obsolete) fragment address */
	__le32 fragment_addr;
	/** @osd2: operating system specific data */
	__le32 osd2[3];
};

/* The header of an ext2 directory entry. */
struct ext2_dirent {
	__le32 inode;
	__le16 direntlen;
	__u8 namelen;
	__u8 filetype;
};

struct ext2fs_node {
	struct ext2_data *data;
	struct ext2_inode inode;
	int ino;
	int inode_read;
};

/* Information about a "mounted" ext2 filesystem. */
struct ext2_data {
	struct ext2_sblock sblock;
	struct ext2_inode *inode;
	struct ext2fs_node diropen;
};

extern lbaint_t part_offset;

int do_ext2ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext2load(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext4_load(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[]);
int do_ext4_ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ext4_write(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
#endif
