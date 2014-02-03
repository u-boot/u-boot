/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * ext4ls and ext4load : Based on ext2 ls and load support in Uboot.
 *		       Ext4 read optimization taken from Open-Moko
 *		       Qi bootloader
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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ext_common.h>
#include <ext4fs.h>
#include "ext4_common.h"

int ext4fs_symlinknest;
struct ext_filesystem ext_fs;

struct ext_filesystem *get_fs(void)
{
	return &ext_fs;
}

void ext4fs_free_node(struct ext2fs_node *node, struct ext2fs_node *currroot)
{
	if ((node != &ext4fs_root->diropen) && (node != currroot))
		free(node);
}

/*
 * Taken from openmoko-kernel mailing list: By Andy green
 * Optimized read file API : collects and defers contiguous sector
 * reads into one potentially more efficient larger sequential read action
 */
int ext4fs_read_file(struct ext2fs_node *node, int pos,
		unsigned int len, char *buf)
{
	struct ext_filesystem *fs = get_fs();
	int i;
	lbaint_t blockcnt;
	int log2blksz = fs->dev_desc->log2blksz;
	int log2_fs_blocksize = LOG2_BLOCK_SIZE(node->data) - log2blksz;
	int blocksize = (1 << (log2_fs_blocksize + log2blksz));
	unsigned int filesize = __le32_to_cpu(node->inode.size);
	lbaint_t previous_block_number = -1;
	lbaint_t delayed_start = 0;
	lbaint_t delayed_extent = 0;
	lbaint_t delayed_skipfirst = 0;
	lbaint_t delayed_next = 0;
	char *delayed_buf = NULL;
	short status;

	/* Adjust len so it we can't read past the end of the file. */
	if (len > filesize)
		len = filesize;

	blockcnt = ((len + pos) + blocksize - 1) / blocksize;

	for (i = pos / blocksize; i < blockcnt; i++) {
		lbaint_t blknr;
		int blockoff = pos % blocksize;
		int blockend = blocksize;
		int skipfirst = 0;
		blknr = read_allocated_block(&(node->inode), i);
		if (blknr < 0)
			return -1;

		blknr = blknr << log2_fs_blocksize;

		/* Last block.  */
		if (i == blockcnt - 1) {
			blockend = (len + pos) % blocksize;

			/* The last portion is exactly blocksize. */
			if (!blockend)
				blockend = blocksize;
		}

		/* First block. */
		if (i == pos / blocksize) {
			skipfirst = blockoff;
			blockend -= skipfirst;
		}
		if (blknr) {
			int status;

			if (previous_block_number != -1) {
				if (delayed_next == blknr) {
					delayed_extent += blockend;
					delayed_next += blockend >> log2blksz;
				} else {	/* spill */
					status = ext4fs_devread(delayed_start,
							delayed_skipfirst,
							delayed_extent,
							delayed_buf);
					if (status == 0)
						return -1;
					previous_block_number = blknr;
					delayed_start = blknr;
					delayed_extent = blockend;
					delayed_skipfirst = skipfirst;
					delayed_buf = buf;
					delayed_next = blknr +
						(blockend >> log2blksz);
				}
			} else {
				previous_block_number = blknr;
				delayed_start = blknr;
				delayed_extent = blockend;
				delayed_skipfirst = skipfirst;
				delayed_buf = buf;
				delayed_next = blknr +
					(blockend >> log2blksz);
			}
		} else {
			if (previous_block_number != -1) {
				/* spill */
				status = ext4fs_devread(delayed_start,
							delayed_skipfirst,
							delayed_extent,
							delayed_buf);
				if (status == 0)
					return -1;
				previous_block_number = -1;
			}
			memset(buf, 0, blocksize - skipfirst);
		}
		buf += blocksize - skipfirst;
	}
	if (previous_block_number != -1) {
		/* spill */
		status = ext4fs_devread(delayed_start,
					delayed_skipfirst, delayed_extent,
					delayed_buf);
		if (status == 0)
			return -1;
		previous_block_number = -1;
	}

	return len;
}

int ext4fs_ls(const char *dirname)
{
	struct ext2fs_node *dirnode;
	int status;

	if (dirname == NULL)
		return 0;

	status = ext4fs_find_file(dirname, &ext4fs_root->diropen, &dirnode,
				  FILETYPE_DIRECTORY);
	if (status != 1) {
		printf("** Can not find directory. **\n");
		return 1;
	}

	ext4fs_iterate_dir(dirnode, NULL, NULL, NULL);
	ext4fs_free_node(dirnode, &ext4fs_root->diropen);

	return 0;
}

int ext4fs_exists(const char *filename)
{
	int file_len;

	file_len = ext4fs_open(filename);
	return file_len >= 0;
}

int ext4fs_read(char *buf, unsigned len)
{
	if (ext4fs_root == NULL || ext4fs_file == NULL)
		return 0;

	return ext4fs_read_file(ext4fs_file, 0, len, buf);
}

int ext4fs_probe(block_dev_desc_t *fs_dev_desc,
		 disk_partition_t *fs_partition)
{
	ext4fs_set_blk_dev(fs_dev_desc, fs_partition);

	if (!ext4fs_mount(fs_partition->size)) {
		ext4fs_close();
		return -1;
	}

	return 0;
}

int ext4_read_file(const char *filename, void *buf, int offset, int len)
{
	int file_len;
	int len_read;

	if (offset != 0) {
		printf("** Cannot support non-zero offset **\n");
		return -1;
	}

	file_len = ext4fs_open(filename);
	if (file_len < 0) {
		printf("** File not found %s **\n", filename);
		return -1;
	}

	if (len == 0)
		len = file_len;

	len_read = ext4fs_read(buf, len);

	return len_read;
}
