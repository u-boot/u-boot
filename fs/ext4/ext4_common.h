/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * ext4ls and ext4load :  based on ext2 ls load support in Uboot.
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * based on code from grub2 fs/ext2.c and fs/fshelp.c by
 * GRUB  --  GRand Unified Bootloader
 * Copyright (C) 2003, 2004  Free Software Foundation, Inc.
 *
 * ext4write : Based on generic ext4 protocol.
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

#ifndef __EXT4_COMMON__
#define __EXT4_COMMON__
#include <ext_common.h>
#include <ext4fs.h>
#include <malloc.h>
#include <asm/errno.h>
#if defined(CONFIG_CMD_EXT4_WRITE)
#include "ext4_journal.h"
#include "crc16.h"
#endif

#define YES		1
#define NO		0
#define TRUE		1
#define FALSE		0
#define RECOVER	1
#define SCAN		0

#define S_IFLNK		0120000		/* symbolic link */
#define BLOCK_NO_ONE		1
#define SUPERBLOCK_SECTOR	2
#define SUPERBLOCK_SIZE	1024
#define F_FILE			1

static inline void *zalloc(size_t size)
{
	void *p = memalign(ARCH_DMA_MINALIGN, size);
	memset(p, 0, size);
	return p;
}

int ext4fs_read_inode(struct ext2_data *data, int ino,
		      struct ext2_inode *inode);
int ext4fs_read_file(struct ext2fs_node *node, int pos,
		unsigned int len, char *buf);
int ext4fs_find_file(const char *path, struct ext2fs_node *rootnode,
			struct ext2fs_node **foundnode, int expecttype);
int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name,
			struct ext2fs_node **fnode, int *ftype);

#if defined(CONFIG_CMD_EXT4_WRITE)
uint32_t ext4fs_div_roundup(uint32_t size, uint32_t n);
int ext4fs_checksum_update(unsigned int i);
int ext4fs_get_parent_inode_num(const char *dirname, char *dname, int flags);
void ext4fs_update_parent_dentry(char *filename, int *p_ino, int file_type);
long int ext4fs_get_new_blk_no(void);
int ext4fs_get_new_inode_no(void);
void ext4fs_reset_block_bmap(long int blockno, unsigned char *buffer,
					int index);
int ext4fs_set_block_bmap(long int blockno, unsigned char *buffer, int index);
int ext4fs_set_inode_bmap(int inode_no, unsigned char *buffer, int index);
void ext4fs_reset_inode_bmap(int inode_no, unsigned char *buffer, int index);
int ext4fs_iget(int inode_no, struct ext2_inode *inode);
void ext4fs_allocate_blocks(struct ext2_inode *file_inode,
				unsigned int total_remaining_blocks,
				unsigned int *total_no_of_block);
void put_ext4(uint64_t off, void *buf, uint32_t size);
#endif
#endif
