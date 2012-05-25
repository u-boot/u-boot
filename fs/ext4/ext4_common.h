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

#define zalloc(size) calloc(1, size)

extern unsigned long part_offset;
int ext4fs_read_inode(struct ext2_data *data, int ino,
		      struct ext2_inode *inode);
int ext4fs_read_file(struct ext2fs_node *node, int pos,
		unsigned int len, char *buf);
int ext4fs_find_file(const char *path, struct ext2fs_node *rootnode,
			struct ext2fs_node **foundnode, int expecttype);
int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name,
			struct ext2fs_node **fnode, int *ftype);
#endif
