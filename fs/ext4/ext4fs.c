// SPDX-License-Identifier: GPL-2.0+
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
 */

#include <blk.h>
#include <div64.h>
#include <errno.h>
#include <ext_common.h>
#include <ext4fs.h>
#include <malloc.h>
#include <part.h>
#include <rtc.h>
#include <u-boot/uuid.h>
#include "ext4_common.h"

int ext4fs_symlinknest;
struct ext_filesystem ext_fs;

/**
 * struct ext4_dir_stream - ext4 directory stream
 *
 * @parent: partition data used by fs layer.
 * This field must be at the beginning of the structure.
 * All other fields are private to the ext4 driver.
 * @root:	root directory node
 * @dir:	directory node
 * @dirent:	directory stream entry
 * @fpos:	file position in directory
 */
struct ext4_dir_stream {
	struct fs_dir_stream parent;
	char *dirname;
	struct fs_dirent dirent;
	unsigned int fpos;
};

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
int ext4fs_read_file(struct ext2fs_node *node, loff_t pos,
		loff_t len, char *buf, loff_t *actread)
{
	struct ext_filesystem *fs = get_fs();
	int i;
	lbaint_t blockcnt;
	int log2blksz = fs->dev_desc->log2blksz;
	int log2_fs_blocksize = LOG2_BLOCK_SIZE(node->data) - log2blksz;
	int blocksize = (1 << (log2_fs_blocksize + log2blksz));
	unsigned int filesize = le32_to_cpu(node->inode.size);
	lbaint_t previous_block_number = -1;
	lbaint_t delayed_start = 0;
	lbaint_t delayed_extent = 0;
	lbaint_t delayed_skipfirst = 0;
	lbaint_t delayed_next = 0;
	char *delayed_buf = NULL;
	char *start_buf = buf;
	short status;
	struct ext_block_cache cache;

	ext_cache_init(&cache);

	/* Adjust len so it we can't read past the end of the file. */
	if (len + pos > filesize)
		len = (filesize - pos);

	if (blocksize <= 0 || len <= 0) {
		ext_cache_fini(&cache);
		return -1;
	}

	blockcnt = lldiv(((len + pos) + blocksize - 1), blocksize);

	for (i = lldiv(pos, blocksize); i < blockcnt; i++) {
		lbaint_t blknr;
		long blknr_and_status;
		int blockoff = pos - (blocksize * i);
		int blockend = blocksize;
		int skipfirst = 0;
		blknr_and_status = read_allocated_block(&node->inode, i, &cache);
		if (blknr_and_status < 0) {
			ext_cache_fini(&cache);
			return -1;
		}

		/* Block number could becomes very large when CONFIG_SYS_64BIT_LBA is enabled
		 * and wrap around at max long int
		 */
		blknr = (lbaint_t)blknr_and_status << log2_fs_blocksize;

		/* Last block.  */
		if (i == blockcnt - 1) {
			blockend = (len + pos) - (blocksize * i);

			/* The last portion is exactly blocksize. */
			if (!blockend)
				blockend = blocksize;
		}

		/* First block. */
		if (i == lldiv(pos, blocksize)) {
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
					if (status == 0) {
						ext_cache_fini(&cache);
						return -1;
					}
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
			int n;
			int n_left;
			if (previous_block_number != -1) {
				/* spill */
				status = ext4fs_devread(delayed_start,
							delayed_skipfirst,
							delayed_extent,
							delayed_buf);
				if (status == 0) {
					ext_cache_fini(&cache);
					return -1;
				}
				previous_block_number = -1;
			}
			/* Zero no more than `len' bytes. */
			n = blocksize - skipfirst;
			n_left = len - ( buf - start_buf );
			if (n > n_left)
				n = n_left;
			memset(buf, 0, n);
		}
		buf += blocksize - skipfirst;
	}
	if (previous_block_number != -1) {
		/* spill */
		status = ext4fs_devread(delayed_start,
					delayed_skipfirst, delayed_extent,
					delayed_buf);
		if (status == 0) {
			ext_cache_fini(&cache);
			return -1;
		}
		previous_block_number = -1;
	}

	*actread  = len;
	ext_cache_fini(&cache);
	return 0;
}

int ext4fs_opendir(const char *dirname, struct fs_dir_stream **dirsp)
{
	struct ext4_dir_stream *dirs;
	struct ext2fs_node *dir = NULL;
	int ret;

	*dirsp = NULL;

	dirs = calloc(1, sizeof(struct ext4_dir_stream));
	if (!dirs)
		return -ENOMEM;
	dirs->dirname = strdup(dirname);
	if (!dirs->dirname) {
		free(dirs);
		return -ENOMEM;
	}

	ret = ext4fs_find_file(dirname, &ext4fs_root->diropen, &dir,
			       FILETYPE_DIRECTORY);
	if (ret == 1) {
		ret = 0;
		*dirsp = (struct fs_dir_stream *)dirs;
	} else {
		free(dirs->dirname);
		free(dirs);
		ret = -ENOENT;
	}

	if (dir)
		ext4fs_free_node(dir, &ext4fs_root->diropen);

	return ret;
}

int ext4fs_readdir(struct fs_dir_stream *fs_dirs, struct fs_dirent **dentp)
{
	struct ext4_dir_stream *dirs = (struct ext4_dir_stream *)fs_dirs;
	struct fs_dirent *dent = &dirs->dirent;
	struct ext2fs_node *dir = NULL;
	int ret;
	loff_t actread;
	struct ext2fs_node fdiro;
	int len;
	struct ext2_dirent dirent;

	*dentp = NULL;

	ret = ext4fs_find_file(dirs->dirname, &ext4fs_root->diropen, &dir,
			       FILETYPE_DIRECTORY);
	if (ret != 1) {
		ret = -ENOENT;
		goto out;
	}
	if (!dir->inode_read) {
		ret = ext4fs_read_inode(dir->data, dir->ino, &dir->inode);
		if (!ret) {
			ret = -EIO;
			goto out;
		}
	}

	if (dirs->fpos >= le32_to_cpu(dir->inode.size))
		return -ENOENT;

	memset(dent, 0, sizeof(struct fs_dirent));

	while (dirs->fpos < le32_to_cpu(dir->inode.size)) {
		ret = ext4fs_read_file(dir, dirs->fpos,
				       sizeof(struct ext2_dirent),
				       (char *)&dirent, &actread);
		if (ret < 0)
			return ret;

		if (!dirent.direntlen)
			return -EIO;

		if (dirent.namelen)
			break;

		dirs->fpos += le16_to_cpu(dirent.direntlen);
	}

	len = min(FS_DIRENT_NAME_LEN - 1, (int)dirent.namelen);

	ret = ext4fs_read_file(dir, dirs->fpos + sizeof(struct ext2_dirent),
			       len, dent->name, &actread);
	if (ret < 0)
		goto out;
	dent->name[len] = '\0';

	fdiro.data = dir->data;
	fdiro.ino = le32_to_cpu(dirent.inode);

	ret = ext4fs_read_inode(dir->data, fdiro.ino, &fdiro.inode);
	if (!ret) {
		ret = -EIO;
		goto out;
	}

	switch (le16_to_cpu(fdiro.inode.mode) & FILETYPE_INO_MASK) {
	case FILETYPE_INO_DIRECTORY:
		dent->type = FS_DT_DIR;
		break;
	case FILETYPE_INO_SYMLINK:
		dent->type = FS_DT_LNK;
		break;
	case FILETYPE_INO_REG:
		dent->type = FS_DT_REG;
		break;
	default:
		dent->type = FILETYPE_UNKNOWN;
	}

	rtc_to_tm(fdiro.inode.atime, &dent->access_time);
	rtc_to_tm(fdiro.inode.ctime, &dent->create_time);
	rtc_to_tm(fdiro.inode.mtime, &dent->change_time);

	dirs->fpos += le16_to_cpu(dirent.direntlen);
	dent->size = fdiro.inode.size;
	*dentp = dent;
	ret = 0;

out:
	if (dir)
		ext4fs_free_node(dir, &ext4fs_root->diropen);

	return ret;
}

void ext4fs_closedir(struct fs_dir_stream *fs_dirs)
{
	struct ext4_dir_stream *dirs = (struct ext4_dir_stream *)fs_dirs;

	if (!dirs)
		return;

	free(dirs->dirname);
	free(dirs);
}

int ext4fs_exists(const char *filename)
{
	struct ext2fs_node *dirnode = NULL;
	int filetype;
	int ret;

	if (!filename)
		return 0;

	ret = ext4fs_find_file1(filename, &ext4fs_root->diropen, &dirnode,
				&filetype);
	if (dirnode)
		ext4fs_free_node(dirnode, &ext4fs_root->diropen);

	return ret;
}

int ext4fs_size(const char *filename, loff_t *size)
{
	return ext4fs_open(filename, size);
}

int ext4fs_read(char *buf, loff_t offset, loff_t len, loff_t *actread)
{
	if (ext4fs_root == NULL || ext4fs_file == NULL)
		return -1;

	return ext4fs_read_file(ext4fs_file, offset, len, buf, actread);
}

int ext4fs_probe(struct blk_desc *fs_dev_desc,
		 struct disk_partition *fs_partition)
{
	ext4fs_set_blk_dev(fs_dev_desc, fs_partition);

	if (!ext4fs_mount()) {
		ext4fs_close();
		return -1;
	}

	return 0;
}

int ext4_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		   loff_t *len_read)
{
	loff_t file_len;
	int ret;

	ret = ext4fs_open(filename, &file_len);
	if (ret < 0) {
		printf("** File not found %s **\n", filename);
		return -1;
	}

	if (len == 0)
		len = file_len;

	return ext4fs_read(buf, offset, len, len_read);
}

int ext4fs_uuid(char *uuid_str)
{
	if (ext4fs_root == NULL)
		return -1;

#ifdef CONFIG_LIB_UUID
	uuid_bin_to_str((unsigned char *)ext4fs_root->sblock.unique_id,
			uuid_str, UUID_STR_FORMAT_STD);

	return 0;
#else
	return -ENOSYS;
#endif
}

void ext_cache_init(struct ext_block_cache *cache)
{
	memset(cache, 0, sizeof(*cache));
}

void ext_cache_fini(struct ext_block_cache *cache)
{
	free(cache->buf);
	ext_cache_init(cache);
}

int ext_cache_read(struct ext_block_cache *cache, lbaint_t block, int size)
{
	/* This could be more lenient, but this is simple and enough for now */
	if (cache->buf && cache->block == block && cache->size == size)
		return 1;
	ext_cache_fini(cache);
	cache->buf = memalign(ARCH_DMA_MINALIGN, size);
	if (!cache->buf)
		return 0;
	if (!ext4fs_devread(block, 0, size, cache->buf)) {
		ext_cache_fini(cache);
		return 0;
	}
	cache->block = block;
	cache->size = size;
	return 1;
}
