/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Journal data structures and headers for Journaling feature of ext4
 * have been referred from JBD2 (Journaling Block device 2)
 * implementation in Linux Kernel.
 *
 * Written by Stephen C. Tweedie <sct@redhat.com>
 *
 * Copyright 1998-2000 Red Hat, Inc --- All Rights Reserved
 * This file is part of the Linux kernel and is made available under
 * the terms of the GNU General Public License, version 2, or at your
 * option, any later version, incorporated herein by reference.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __EXT4_JRNL__
#define __EXT4_JRNL__

#define EXT2_JOURNAL_INO		8	/* Journal inode */
#define EXT2_JOURNAL_SUPERBLOCK	0	/* Journal  Superblock number */

#define JBD2_FEATURE_COMPAT_CHECKSUM	0x00000001
#define EXT3_JOURNAL_MAGIC_NUMBER	0xc03b3998U
#define TRANSACTION_RUNNING		1
#define TRANSACTION_COMPLETE		0
#define EXT3_FEATURE_INCOMPAT_RECOVER	0x0004	/* Needs recovery */
#define EXT3_JOURNAL_DESCRIPTOR_BLOCK	1
#define EXT3_JOURNAL_COMMIT_BLOCK	2
#define EXT3_JOURNAL_SUPERBLOCK_V1	3
#define EXT3_JOURNAL_SUPERBLOCK_V2	4
#define EXT3_JOURNAL_REVOKE_BLOCK	5
#define EXT3_JOURNAL_FLAG_ESCAPE	1
#define EXT3_JOURNAL_FLAG_SAME_UUID	2
#define EXT3_JOURNAL_FLAG_DELETED	4
#define EXT3_JOURNAL_FLAG_LAST_TAG	8

/* Maximum entries in 1 journal transaction */
#define MAX_JOURNAL_ENTRIES 100
struct journal_log {
	char *buf;
	int blknr;
};

struct dirty_blocks {
	char *buf;
	int blknr;
};

/* Standard header for all descriptor blocks: */
struct journal_header_t {
	__u32 h_magic;
	__u32 h_blocktype;
	__u32 h_sequence;
};

/* The journal superblock.  All fields are in big-endian byte order. */
struct journal_superblock_t {
	/* 0x0000 */
	struct journal_header_t s_header;

	/* Static information describing the journal */
	__u32 s_blocksize;	/* journal device blocksize */
	__u32 s_maxlen;		/* total blocks in journal file */
	__u32 s_first;		/* first block of log information */

	/* Dynamic information describing the current state of the log */
	__u32 s_sequence;	/* first commit ID expected in log */
	__u32 s_start;		/* blocknr of start of log */

	/* Error value, as set by journal_abort(). */
	__s32 s_errno;

	/* Remaining fields are only valid in a version-2 superblock */
	__u32 s_feature_compat;	/* compatible feature set */
	__u32 s_feature_incompat;	/* incompatible feature set */
	__u32 s_feature_ro_compat;	/* readonly-compatible feature set */
	/* 0x0030 */
	__u8 s_uuid[16];	/* 128-bit uuid for journal */

	/* 0x0040 */
	__u32 s_nr_users;	/* Nr of filesystems sharing log */

	__u32 s_dynsuper;	/* Blocknr of dynamic superblock copy */

	/* 0x0048 */
	__u32 s_max_transaction;	/* Limit of journal blocks per trans. */
	__u32 s_max_trans_data;	/* Limit of data blocks per trans. */

	/* 0x0050 */
	__u32 s_padding[44];

	/* 0x0100 */
	__u8 s_users[16 * 48];	/* ids of all fs'es sharing the log */
	/* 0x0400 */
} ;

struct ext3_journal_block_tag {
	uint32_t block;
	uint32_t flags;
};

struct journal_revoke_header_t {
	struct journal_header_t r_header;
	int r_count;		/* Count of bytes used in the block */
};

struct revoke_blk_list {
	char *content;		/* revoke block itself */
	struct revoke_blk_list *next;
};

extern struct ext2_data *ext4fs_root;

int ext4fs_init_journal(void);
int ext4fs_log_gdt(char *gd_table);
int ext4fs_check_journal_state(int recovery_flag);
int ext4fs_log_journal(char *journal_buffer, long int blknr);
int ext4fs_put_metadata(char *metadata_buffer, long int blknr);
void ext4fs_update_journal(void);
void ext4fs_dump_metadata(void);
void ext4fs_push_revoke_blk(char *buffer);
void ext4fs_free_journal(void);
void ext4fs_free_revoke_blks(void);
#endif
