/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * ext4ls and ext4load : Based on ext2 ls load support in Uboot.
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
#include <inttypes.h>
#include <malloc.h>
#include <memalign.h>
#include <stddef.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <asm/byteorder.h>
#include "ext4_common.h"

struct ext2_data *ext4fs_root;
struct ext2fs_node *ext4fs_file;
uint32_t *ext4fs_indir1_block;
int ext4fs_indir1_size;
int ext4fs_indir1_blkno = -1;
uint32_t *ext4fs_indir2_block;
int ext4fs_indir2_size;
int ext4fs_indir2_blkno = -1;

uint32_t *ext4fs_indir3_block;
int ext4fs_indir3_size;
int ext4fs_indir3_blkno = -1;
struct ext2_inode *g_parent_inode;
static int symlinknest;

#if defined(CONFIG_EXT4_WRITE)
uint32_t ext4fs_div_roundup(uint32_t size, uint32_t n)
{
	uint32_t res = size / n;
	if (res * n != size)
		res++;

	return res;
}

void put_ext4(uint64_t off, void *buf, uint32_t size)
{
	uint64_t startblock;
	uint64_t remainder;
	unsigned char *temp_ptr = NULL;
	struct ext_filesystem *fs = get_fs();
	int log2blksz = fs->dev_desc->log2blksz;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, sec_buf, fs->dev_desc->blksz);

	startblock = off >> log2blksz;
	startblock += part_offset;
	remainder = off & (uint64_t)(fs->dev_desc->blksz - 1);

	if (fs->dev_desc == NULL)
		return;

	if ((startblock + (size >> log2blksz)) >
	    (part_offset + fs->total_sect)) {
		printf("part_offset is " LBAFU "\n", part_offset);
		printf("total_sector is %" PRIu64 "\n", fs->total_sect);
		printf("error: overflow occurs\n");
		return;
	}

	if (remainder) {
		blk_dread(fs->dev_desc, startblock, 1, sec_buf);
		temp_ptr = sec_buf;
		memcpy((temp_ptr + remainder), (unsigned char *)buf, size);
		blk_dwrite(fs->dev_desc, startblock, 1, sec_buf);
	} else {
		if (size >> log2blksz != 0) {
			blk_dwrite(fs->dev_desc, startblock, size >> log2blksz,
				   (unsigned long *)buf);
		} else {
			blk_dread(fs->dev_desc, startblock, 1, sec_buf);
			temp_ptr = sec_buf;
			memcpy(temp_ptr, buf, size);
			blk_dwrite(fs->dev_desc, startblock, 1,
				   (unsigned long *)sec_buf);
		}
	}
}

static int _get_new_inode_no(unsigned char *buffer)
{
	struct ext_filesystem *fs = get_fs();
	unsigned char input;
	int operand, status;
	int count = 1;
	int j = 0;

	/* get the blocksize of the filesystem */
	unsigned char *ptr = buffer;
	while (*ptr == 255) {
		ptr++;
		count += 8;
		if (count > ext4fs_root->sblock.inodes_per_group)
			return -1;
	}

	for (j = 0; j < fs->blksz; j++) {
		input = *ptr;
		int i = 0;
		while (i <= 7) {
			operand = 1 << i;
			status = input & operand;
			if (status) {
				i++;
				count++;
			} else {
				*ptr |= operand;
				return count;
			}
		}
		ptr = ptr + 1;
	}

	return -1;
}

static int _get_new_blk_no(unsigned char *buffer)
{
	unsigned char input;
	int operand, status;
	int count = 0;
	int j = 0;
	unsigned char *ptr = buffer;
	struct ext_filesystem *fs = get_fs();

	if (fs->blksz != 1024)
		count = 0;
	else
		count = 1;

	while (*ptr == 255) {
		ptr++;
		count += 8;
		if (count == (fs->blksz * 8))
			return -1;
	}

	for (j = 0; j < fs->blksz; j++) {
		input = *ptr;
		int i = 0;
		while (i <= 7) {
			operand = 1 << i;
			status = input & operand;
			if (status) {
				i++;
				count++;
			} else {
				*ptr |= operand;
				return count;
			}
		}
		ptr = ptr + 1;
	}

	return -1;
}

int ext4fs_set_block_bmap(long int blockno, unsigned char *buffer, int index)
{
	int i, remainder, status;
	unsigned char *ptr = buffer;
	unsigned char operand;
	i = blockno / 8;
	remainder = blockno % 8;
	int blocksize = EXT2_BLOCK_SIZE(ext4fs_root);

	i = i - (index * blocksize);
	if (blocksize != 1024) {
		ptr = ptr + i;
		operand = 1 << remainder;
		status = *ptr & operand;
		if (status)
			return -1;

		*ptr = *ptr | operand;
		return 0;
	} else {
		if (remainder == 0) {
			ptr = ptr + i - 1;
			operand = (1 << 7);
		} else {
			ptr = ptr + i;
			operand = (1 << (remainder - 1));
		}
		status = *ptr & operand;
		if (status)
			return -1;

		*ptr = *ptr | operand;
		return 0;
	}
}

void ext4fs_reset_block_bmap(long int blockno, unsigned char *buffer, int index)
{
	int i, remainder, status;
	unsigned char *ptr = buffer;
	unsigned char operand;
	i = blockno / 8;
	remainder = blockno % 8;
	int blocksize = EXT2_BLOCK_SIZE(ext4fs_root);

	i = i - (index * blocksize);
	if (blocksize != 1024) {
		ptr = ptr + i;
		operand = (1 << remainder);
		status = *ptr & operand;
		if (status)
			*ptr = *ptr & ~(operand);
	} else {
		if (remainder == 0) {
			ptr = ptr + i - 1;
			operand = (1 << 7);
		} else {
			ptr = ptr + i;
			operand = (1 << (remainder - 1));
		}
		status = *ptr & operand;
		if (status)
			*ptr = *ptr & ~(operand);
	}
}

int ext4fs_set_inode_bmap(int inode_no, unsigned char *buffer, int index)
{
	int i, remainder, status;
	unsigned char *ptr = buffer;
	unsigned char operand;

	inode_no -= (index * ext4fs_root->sblock.inodes_per_group);
	i = inode_no / 8;
	remainder = inode_no % 8;
	if (remainder == 0) {
		ptr = ptr + i - 1;
		operand = (1 << 7);
	} else {
		ptr = ptr + i;
		operand = (1 << (remainder - 1));
	}
	status = *ptr & operand;
	if (status)
		return -1;

	*ptr = *ptr | operand;

	return 0;
}

void ext4fs_reset_inode_bmap(int inode_no, unsigned char *buffer, int index)
{
	int i, remainder, status;
	unsigned char *ptr = buffer;
	unsigned char operand;

	inode_no -= (index * ext4fs_root->sblock.inodes_per_group);
	i = inode_no / 8;
	remainder = inode_no % 8;
	if (remainder == 0) {
		ptr = ptr + i - 1;
		operand = (1 << 7);
	} else {
		ptr = ptr + i;
		operand = (1 << (remainder - 1));
	}
	status = *ptr & operand;
	if (status)
		*ptr = *ptr & ~(operand);
}

int ext4fs_checksum_update(unsigned int i)
{
	struct ext2_block_group *desc;
	struct ext_filesystem *fs = get_fs();
	__u16 crc = 0;

	desc = (struct ext2_block_group *)&fs->bgd[i];
	if (fs->sb->feature_ro_compat & EXT4_FEATURE_RO_COMPAT_GDT_CSUM) {
		int offset = offsetof(struct ext2_block_group, bg_checksum);

		crc = ext2fs_crc16(~0, fs->sb->unique_id,
				   sizeof(fs->sb->unique_id));
		crc = ext2fs_crc16(crc, &i, sizeof(i));
		crc = ext2fs_crc16(crc, desc, offset);
		offset += sizeof(desc->bg_checksum);	/* skip checksum */
		assert(offset == sizeof(*desc));
	}

	return crc;
}

static int check_void_in_dentry(struct ext2_dirent *dir, char *filename)
{
	int dentry_length;
	int sizeof_void_space;
	int new_entry_byte_reqd;
	short padding_factor = 0;

	if (dir->namelen % 4 != 0)
		padding_factor = 4 - (dir->namelen % 4);

	dentry_length = sizeof(struct ext2_dirent) +
			dir->namelen + padding_factor;
	sizeof_void_space = dir->direntlen - dentry_length;
	if (sizeof_void_space == 0)
		return 0;

	padding_factor = 0;
	if (strlen(filename) % 4 != 0)
		padding_factor = 4 - (strlen(filename) % 4);

	new_entry_byte_reqd = strlen(filename) +
	    sizeof(struct ext2_dirent) + padding_factor;
	if (sizeof_void_space >= new_entry_byte_reqd) {
		dir->direntlen = dentry_length;
		return sizeof_void_space;
	}

	return 0;
}

void ext4fs_update_parent_dentry(char *filename, int *p_ino, int file_type)
{
	unsigned int *zero_buffer = NULL;
	char *root_first_block_buffer = NULL;
	int direct_blk_idx;
	long int root_blknr;
	long int first_block_no_of_root = 0;
	long int previous_blknr = -1;
	int totalbytes = 0;
	short int padding_factor = 0;
	unsigned int new_entry_byte_reqd;
	unsigned int last_entry_dirlen;
	int sizeof_void_space = 0;
	int templength = 0;
	int inodeno;
	int status;
	struct ext_filesystem *fs = get_fs();
	/* directory entry */
	struct ext2_dirent *dir;
	char *temp_dir = NULL;

	zero_buffer = zalloc(fs->blksz);
	if (!zero_buffer) {
		printf("No Memory\n");
		return;
	}
	root_first_block_buffer = zalloc(fs->blksz);
	if (!root_first_block_buffer) {
		free(zero_buffer);
		printf("No Memory\n");
		return;
	}
restart:

	/* read the block no allocated to a file */
	for (direct_blk_idx = 0; direct_blk_idx < INDIRECT_BLOCKS;
	     direct_blk_idx++) {
		root_blknr = read_allocated_block(g_parent_inode,
						  direct_blk_idx);
		if (root_blknr == 0) {
			first_block_no_of_root = previous_blknr;
			break;
		}
		previous_blknr = root_blknr;
	}

	status = ext4fs_devread((lbaint_t)first_block_no_of_root
				* fs->sect_perblk,
				0, fs->blksz, root_first_block_buffer);
	if (status == 0)
		goto fail;

	if (ext4fs_log_journal(root_first_block_buffer, first_block_no_of_root))
		goto fail;
	dir = (struct ext2_dirent *)root_first_block_buffer;
	totalbytes = 0;
	while (dir->direntlen > 0) {
		/*
		 * blocksize-totalbytes because last directory length
		 * i.e. dir->direntlen is free availble space in the
		 * block that means  it is a last entry of directory
		 * entry
		 */

		/* traversing the each directory entry */
		if (fs->blksz - totalbytes == dir->direntlen) {
			if (strlen(filename) % 4 != 0)
				padding_factor = 4 - (strlen(filename) % 4);

			new_entry_byte_reqd = strlen(filename) +
			    sizeof(struct ext2_dirent) + padding_factor;
			padding_factor = 0;
			/*
			 * update last directory entry length to its
			 * length because we are creating new directory
			 * entry
			 */
			if (dir->namelen % 4 != 0)
				padding_factor = 4 - (dir->namelen % 4);

			last_entry_dirlen = dir->namelen +
			    sizeof(struct ext2_dirent) + padding_factor;
			if ((fs->blksz - totalbytes - last_entry_dirlen) <
				new_entry_byte_reqd) {
				printf("1st Block Full:Allocate new block\n");

				if (direct_blk_idx == INDIRECT_BLOCKS - 1) {
					printf("Directory exceeds limit\n");
					goto fail;
				}
				g_parent_inode->b.blocks.dir_blocks
				    [direct_blk_idx] = ext4fs_get_new_blk_no();
				if (g_parent_inode->b.blocks.dir_blocks
					[direct_blk_idx] == -1) {
					printf("no block left to assign\n");
					goto fail;
				}
				put_ext4(((uint64_t)
					  ((uint64_t)g_parent_inode->b.
					   blocks.dir_blocks[direct_blk_idx] *
					   (uint64_t)fs->blksz)), zero_buffer, fs->blksz);
				g_parent_inode->size =
				    g_parent_inode->size + fs->blksz;
				g_parent_inode->blockcnt =
				    g_parent_inode->blockcnt + fs->sect_perblk;
				if (ext4fs_put_metadata
				    (root_first_block_buffer,
				     first_block_no_of_root))
					goto fail;
				goto restart;
			}
			dir->direntlen = last_entry_dirlen;
			break;
		}

		templength = dir->direntlen;
		totalbytes = totalbytes + templength;
		sizeof_void_space = check_void_in_dentry(dir, filename);
		if (sizeof_void_space)
			break;

		dir = (struct ext2_dirent *)((char *)dir + templength);
	}

	/* make a pointer ready for creating next directory entry */
	templength = dir->direntlen;
	totalbytes = totalbytes + templength;
	dir = (struct ext2_dirent *)((char *)dir + templength);

	/* get the next available inode number */
	inodeno = ext4fs_get_new_inode_no();
	if (inodeno == -1) {
		printf("no inode left to assign\n");
		goto fail;
	}
	dir->inode = inodeno;
	if (sizeof_void_space)
		dir->direntlen = sizeof_void_space;
	else
		dir->direntlen = fs->blksz - totalbytes;

	dir->namelen = strlen(filename);
	dir->filetype = FILETYPE_REG;	/* regular file */
	temp_dir = (char *)dir;
	temp_dir = temp_dir + sizeof(struct ext2_dirent);
	memcpy(temp_dir, filename, strlen(filename));

	*p_ino = inodeno;

	/* update or write  the 1st block of root inode */
	if (ext4fs_put_metadata(root_first_block_buffer,
				first_block_no_of_root))
		goto fail;

fail:
	free(zero_buffer);
	free(root_first_block_buffer);
}

static int search_dir(struct ext2_inode *parent_inode, char *dirname)
{
	int status;
	int inodeno;
	int totalbytes;
	int templength;
	int direct_blk_idx;
	long int blknr;
	int found = 0;
	char *ptr = NULL;
	unsigned char *block_buffer = NULL;
	struct ext2_dirent *dir = NULL;
	struct ext2_dirent *previous_dir = NULL;
	struct ext_filesystem *fs = get_fs();

	/* read the block no allocated to a file */
	for (direct_blk_idx = 0; direct_blk_idx < INDIRECT_BLOCKS;
		direct_blk_idx++) {
		blknr = read_allocated_block(parent_inode, direct_blk_idx);
		if (blknr == 0)
			goto fail;

		/* read the blocks of parenet inode */
		block_buffer = zalloc(fs->blksz);
		if (!block_buffer)
			goto fail;

		status = ext4fs_devread((lbaint_t)blknr * fs->sect_perblk,
					0, fs->blksz, (char *)block_buffer);
		if (status == 0)
			goto fail;

		dir = (struct ext2_dirent *)block_buffer;
		ptr = (char *)dir;
		totalbytes = 0;
		while (dir->direntlen >= 0) {
			/*
			 * blocksize-totalbytes because last directory
			 * length i.e.,*dir->direntlen is free availble
			 * space in the block that means
			 * it is a last entry of directory entry
			 */
			if (strlen(dirname) == dir->namelen) {
				if (strncmp(dirname, ptr +
					sizeof(struct ext2_dirent),
					dir->namelen) == 0) {
					previous_dir->direntlen +=
							dir->direntlen;
					inodeno = dir->inode;
					dir->inode = 0;
					found = 1;
					break;
				}
			}

			if (fs->blksz - totalbytes == dir->direntlen)
				break;

			/* traversing the each directory entry */
			templength = dir->direntlen;
			totalbytes = totalbytes + templength;
			previous_dir = dir;
			dir = (struct ext2_dirent *)((char *)dir + templength);
			ptr = (char *)dir;
		}

		if (found == 1) {
			free(block_buffer);
			block_buffer = NULL;
			return inodeno;
		}

		free(block_buffer);
		block_buffer = NULL;
	}

fail:
	free(block_buffer);

	return -1;
}

static int find_dir_depth(char *dirname)
{
	char *token = strtok(dirname, "/");
	int count = 0;
	while (token != NULL) {
		token = strtok(NULL, "/");
		count++;
	}
	return count + 1 + 1;
	/*
	 * for example  for string /home/temp
	 * depth=home(1)+temp(1)+1 extra for NULL;
	 * so count is 4;
	 */
}

static int parse_path(char **arr, char *dirname)
{
	char *token = strtok(dirname, "/");
	int i = 0;

	/* add root */
	arr[i] = zalloc(strlen("/") + 1);
	if (!arr[i])
		return -ENOMEM;
	memcpy(arr[i++], "/", strlen("/"));

	/* add each path entry after root */
	while (token != NULL) {
		arr[i] = zalloc(strlen(token) + 1);
		if (!arr[i])
			return -ENOMEM;
		memcpy(arr[i++], token, strlen(token));
		token = strtok(NULL, "/");
	}
	arr[i] = NULL;

	return 0;
}

int ext4fs_iget(int inode_no, struct ext2_inode *inode)
{
	if (ext4fs_read_inode(ext4fs_root, inode_no, inode) == 0)
		return -1;

	return 0;
}

/*
 * Function: ext4fs_get_parent_inode_num
 * Return Value: inode Number of the parent directory of  file/Directory to be
 * created
 * dirname : Input parmater, input path name of the file/directory to be created
 * dname : Output parameter, to be filled with the name of the directory
 * extracted from dirname
 */
int ext4fs_get_parent_inode_num(const char *dirname, char *dname, int flags)
{
	int i;
	int depth = 0;
	int matched_inode_no;
	int result_inode_no = -1;
	char **ptr = NULL;
	char *depth_dirname = NULL;
	char *parse_dirname = NULL;
	struct ext2_inode *parent_inode = NULL;
	struct ext2_inode *first_inode = NULL;
	struct ext2_inode temp_inode;

	if (*dirname != '/') {
		printf("Please supply Absolute path\n");
		return -1;
	}

	/* TODO: input validation make equivalent to linux */
	depth_dirname = zalloc(strlen(dirname) + 1);
	if (!depth_dirname)
		return -ENOMEM;

	memcpy(depth_dirname, dirname, strlen(dirname));
	depth = find_dir_depth(depth_dirname);
	parse_dirname = zalloc(strlen(dirname) + 1);
	if (!parse_dirname)
		goto fail;
	memcpy(parse_dirname, dirname, strlen(dirname));

	/* allocate memory for each directory level */
	ptr = zalloc((depth) * sizeof(char *));
	if (!ptr)
		goto fail;
	if (parse_path(ptr, parse_dirname))
		goto fail;
	parent_inode = zalloc(sizeof(struct ext2_inode));
	if (!parent_inode)
		goto fail;
	first_inode = zalloc(sizeof(struct ext2_inode));
	if (!first_inode)
		goto fail;
	memcpy(parent_inode, ext4fs_root->inode, sizeof(struct ext2_inode));
	memcpy(first_inode, parent_inode, sizeof(struct ext2_inode));
	if (flags & F_FILE)
		result_inode_no = EXT2_ROOT_INO;
	for (i = 1; i < depth; i++) {
		matched_inode_no = search_dir(parent_inode, ptr[i]);
		if (matched_inode_no == -1) {
			if (ptr[i + 1] == NULL && i == 1) {
				result_inode_no = EXT2_ROOT_INO;
				goto end;
			} else {
				if (ptr[i + 1] == NULL)
					break;
				printf("Invalid path\n");
				result_inode_no = -1;
				goto fail;
			}
		} else {
			if (ptr[i + 1] != NULL) {
				memset(parent_inode, '\0',
				       sizeof(struct ext2_inode));
				if (ext4fs_iget(matched_inode_no,
						parent_inode)) {
					result_inode_no = -1;
					goto fail;
				}
				result_inode_no = matched_inode_no;
			} else {
				break;
			}
		}
	}

end:
	if (i == 1)
		matched_inode_no = search_dir(first_inode, ptr[i]);
	else
		matched_inode_no = search_dir(parent_inode, ptr[i]);

	if (matched_inode_no != -1) {
		ext4fs_iget(matched_inode_no, &temp_inode);
		if (temp_inode.mode & S_IFDIR) {
			printf("It is a Directory\n");
			result_inode_no = -1;
			goto fail;
		}
	}

	if (strlen(ptr[i]) > 256) {
		result_inode_no = -1;
		goto fail;
	}
	memcpy(dname, ptr[i], strlen(ptr[i]));

fail:
	free(depth_dirname);
	free(parse_dirname);
	for (i = 0; i < depth; i++) {
		if (!ptr[i])
			break;
		free(ptr[i]);
	}
	free(ptr);
	free(parent_inode);
	free(first_inode);

	return result_inode_no;
}

static int check_filename(char *filename, unsigned int blknr)
{
	unsigned int first_block_no_of_root;
	int totalbytes = 0;
	int templength = 0;
	int status, inodeno;
	int found = 0;
	char *root_first_block_buffer = NULL;
	char *root_first_block_addr = NULL;
	struct ext2_dirent *dir = NULL;
	struct ext2_dirent *previous_dir = NULL;
	char *ptr = NULL;
	struct ext_filesystem *fs = get_fs();
	int ret = -1;

	/* get the first block of root */
	first_block_no_of_root = blknr;
	root_first_block_buffer = zalloc(fs->blksz);
	if (!root_first_block_buffer)
		return -ENOMEM;
	root_first_block_addr = root_first_block_buffer;
	status = ext4fs_devread((lbaint_t)first_block_no_of_root *
				fs->sect_perblk, 0,
				fs->blksz, root_first_block_buffer);
	if (status == 0)
		goto fail;

	if (ext4fs_log_journal(root_first_block_buffer, first_block_no_of_root))
		goto fail;
	dir = (struct ext2_dirent *)root_first_block_buffer;
	ptr = (char *)dir;
	totalbytes = 0;
	while (dir->direntlen >= 0) {
		/*
		 * blocksize-totalbytes because last
		 * directory length i.e., *dir->direntlen
		 * is free availble space in the block that
		 * means it is a last entry of directory entry
		 */
		if (strlen(filename) == dir->namelen) {
			if (strncmp(filename, ptr + sizeof(struct ext2_dirent),
				dir->namelen) == 0) {
				printf("file found deleting\n");
				previous_dir->direntlen += dir->direntlen;
				inodeno = dir->inode;
				dir->inode = 0;
				found = 1;
				break;
			}
		}

		if (fs->blksz - totalbytes == dir->direntlen)
			break;

		/* traversing the each directory entry */
		templength = dir->direntlen;
		totalbytes = totalbytes + templength;
		previous_dir = dir;
		dir = (struct ext2_dirent *)((char *)dir + templength);
		ptr = (char *)dir;
	}


	if (found == 1) {
		if (ext4fs_put_metadata(root_first_block_addr,
					first_block_no_of_root))
			goto fail;
		ret = inodeno;
	}
fail:
	free(root_first_block_buffer);

	return ret;
}

int ext4fs_filename_check(char *filename)
{
	short direct_blk_idx = 0;
	long int blknr = -1;
	int inodeno = -1;

	/* read the block no allocated to a file */
	for (direct_blk_idx = 0; direct_blk_idx < INDIRECT_BLOCKS;
		direct_blk_idx++) {
		blknr = read_allocated_block(g_parent_inode, direct_blk_idx);
		if (blknr == 0)
			break;
		inodeno = check_filename(filename, blknr);
		if (inodeno != -1)
			return inodeno;
	}

	return -1;
}

long int ext4fs_get_new_blk_no(void)
{
	short i;
	short status;
	int remainder;
	unsigned int bg_idx;
	static int prev_bg_bitmap_index = -1;
	unsigned int blk_per_grp = ext4fs_root->sblock.blocks_per_group;
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	char *zero_buffer = zalloc(fs->blksz);
	if (!journal_buffer || !zero_buffer)
		goto fail;
	struct ext2_block_group *bgd = (struct ext2_block_group *)fs->gdtable;

	if (fs->first_pass_bbmap == 0) {
		for (i = 0; i < fs->no_blkgrp; i++) {
			if (bgd[i].free_blocks) {
				if (bgd[i].bg_flags & EXT4_BG_BLOCK_UNINIT) {
					put_ext4(((uint64_t) ((uint64_t)bgd[i].block_id *
							      (uint64_t)fs->blksz)),
						 zero_buffer, fs->blksz);
					bgd[i].bg_flags =
					    bgd[i].
					    bg_flags & ~EXT4_BG_BLOCK_UNINIT;
					memcpy(fs->blk_bmaps[i], zero_buffer,
					       fs->blksz);
				}
				fs->curr_blkno =
				    _get_new_blk_no(fs->blk_bmaps[i]);
				if (fs->curr_blkno == -1)
					/* if block bitmap is completely fill */
					continue;
				fs->curr_blkno = fs->curr_blkno +
						(i * fs->blksz * 8);
				fs->first_pass_bbmap++;
				bgd[i].free_blocks--;
				fs->sb->free_blocks--;
				status = ext4fs_devread((lbaint_t)
							bgd[i].block_id *
							fs->sect_perblk, 0,
							fs->blksz,
							journal_buffer);
				if (status == 0)
					goto fail;
				if (ext4fs_log_journal(journal_buffer,
							bgd[i].block_id))
					goto fail;
				goto success;
			} else {
				debug("no space left on block group %d\n", i);
			}
		}

		goto fail;
	} else {
restart:
		fs->curr_blkno++;
		/* get the blockbitmap index respective to blockno */
		bg_idx = fs->curr_blkno / blk_per_grp;
		if (fs->blksz == 1024) {
			remainder = fs->curr_blkno % blk_per_grp;
			if (!remainder)
				bg_idx--;
		}

		/*
		 * To skip completely filled block group bitmaps
		 * Optimize the block allocation
		 */
		if (bg_idx >= fs->no_blkgrp)
			goto fail;

		if (bgd[bg_idx].free_blocks == 0) {
			debug("block group %u is full. Skipping\n", bg_idx);
			fs->curr_blkno = fs->curr_blkno + blk_per_grp;
			fs->curr_blkno--;
			goto restart;
		}

		if (bgd[bg_idx].bg_flags & EXT4_BG_BLOCK_UNINIT) {
			memset(zero_buffer, '\0', fs->blksz);
			put_ext4(((uint64_t) ((uint64_t)bgd[bg_idx].block_id *
					(uint64_t)fs->blksz)), zero_buffer, fs->blksz);
			memcpy(fs->blk_bmaps[bg_idx], zero_buffer, fs->blksz);
			bgd[bg_idx].bg_flags = bgd[bg_idx].bg_flags &
						~EXT4_BG_BLOCK_UNINIT;
		}

		if (ext4fs_set_block_bmap(fs->curr_blkno, fs->blk_bmaps[bg_idx],
				   bg_idx) != 0) {
			debug("going for restart for the block no %ld %u\n",
			      fs->curr_blkno, bg_idx);
			goto restart;
		}

		/* journal backup */
		if (prev_bg_bitmap_index != bg_idx) {
			memset(journal_buffer, '\0', fs->blksz);
			status = ext4fs_devread((lbaint_t)bgd[bg_idx].block_id
						* fs->sect_perblk,
						0, fs->blksz, journal_buffer);
			if (status == 0)
				goto fail;
			if (ext4fs_log_journal(journal_buffer,
						bgd[bg_idx].block_id))
				goto fail;

			prev_bg_bitmap_index = bg_idx;
		}
		bgd[bg_idx].free_blocks--;
		fs->sb->free_blocks--;
		goto success;
	}
success:
	free(journal_buffer);
	free(zero_buffer);

	return fs->curr_blkno;
fail:
	free(journal_buffer);
	free(zero_buffer);

	return -1;
}

int ext4fs_get_new_inode_no(void)
{
	short i;
	short status;
	unsigned int ibmap_idx;
	static int prev_inode_bitmap_index = -1;
	unsigned int inodes_per_grp = ext4fs_root->sblock.inodes_per_group;
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	char *zero_buffer = zalloc(fs->blksz);
	if (!journal_buffer || !zero_buffer)
		goto fail;
	struct ext2_block_group *bgd = (struct ext2_block_group *)fs->gdtable;

	if (fs->first_pass_ibmap == 0) {
		for (i = 0; i < fs->no_blkgrp; i++) {
			if (bgd[i].free_inodes) {
				if (bgd[i].bg_itable_unused !=
						bgd[i].free_inodes)
					bgd[i].bg_itable_unused =
						bgd[i].free_inodes;
				if (bgd[i].bg_flags & EXT4_BG_INODE_UNINIT) {
					put_ext4(((uint64_t)
						  ((uint64_t)bgd[i].inode_id *
							(uint64_t)fs->blksz)),
						 zero_buffer, fs->blksz);
					bgd[i].bg_flags = bgd[i].bg_flags &
							~EXT4_BG_INODE_UNINIT;
					memcpy(fs->inode_bmaps[i],
					       zero_buffer, fs->blksz);
				}
				fs->curr_inode_no =
				    _get_new_inode_no(fs->inode_bmaps[i]);
				if (fs->curr_inode_no == -1)
					/* if block bitmap is completely fill */
					continue;
				fs->curr_inode_no = fs->curr_inode_no +
							(i * inodes_per_grp);
				fs->first_pass_ibmap++;
				bgd[i].free_inodes--;
				bgd[i].bg_itable_unused--;
				fs->sb->free_inodes--;
				status = ext4fs_devread((lbaint_t)
							bgd[i].inode_id *
							fs->sect_perblk, 0,
							fs->blksz,
							journal_buffer);
				if (status == 0)
					goto fail;
				if (ext4fs_log_journal(journal_buffer,
							bgd[i].inode_id))
					goto fail;
				goto success;
			} else
				debug("no inode left on block group %d\n", i);
		}
		goto fail;
	} else {
restart:
		fs->curr_inode_no++;
		/* get the blockbitmap index respective to blockno */
		ibmap_idx = fs->curr_inode_no / inodes_per_grp;
		if (bgd[ibmap_idx].bg_flags & EXT4_BG_INODE_UNINIT) {
			memset(zero_buffer, '\0', fs->blksz);
			put_ext4(((uint64_t) ((uint64_t)bgd[ibmap_idx].inode_id *
					      (uint64_t)fs->blksz)), zero_buffer,
				 fs->blksz);
			bgd[ibmap_idx].bg_flags =
			    bgd[ibmap_idx].bg_flags & ~EXT4_BG_INODE_UNINIT;
			memcpy(fs->inode_bmaps[ibmap_idx], zero_buffer,
				fs->blksz);
		}

		if (ext4fs_set_inode_bmap(fs->curr_inode_no,
					  fs->inode_bmaps[ibmap_idx],
					  ibmap_idx) != 0) {
			debug("going for restart for the block no %d %u\n",
			      fs->curr_inode_no, ibmap_idx);
			goto restart;
		}

		/* journal backup */
		if (prev_inode_bitmap_index != ibmap_idx) {
			memset(journal_buffer, '\0', fs->blksz);
			status = ext4fs_devread((lbaint_t)
						bgd[ibmap_idx].inode_id
						* fs->sect_perblk,
						0, fs->blksz, journal_buffer);
			if (status == 0)
				goto fail;
			if (ext4fs_log_journal(journal_buffer,
						bgd[ibmap_idx].inode_id))
				goto fail;
			prev_inode_bitmap_index = ibmap_idx;
		}
		if (bgd[ibmap_idx].bg_itable_unused !=
				bgd[ibmap_idx].free_inodes)
			bgd[ibmap_idx].bg_itable_unused =
					bgd[ibmap_idx].free_inodes;
		bgd[ibmap_idx].free_inodes--;
		bgd[ibmap_idx].bg_itable_unused--;
		fs->sb->free_inodes--;
		goto success;
	}

success:
	free(journal_buffer);
	free(zero_buffer);

	return fs->curr_inode_no;
fail:
	free(journal_buffer);
	free(zero_buffer);

	return -1;

}


static void alloc_single_indirect_block(struct ext2_inode *file_inode,
					unsigned int *total_remaining_blocks,
					unsigned int *no_blks_reqd)
{
	short i;
	short status;
	long int actual_block_no;
	long int si_blockno;
	/* si :single indirect */
	unsigned int *si_buffer = NULL;
	unsigned int *si_start_addr = NULL;
	struct ext_filesystem *fs = get_fs();

	if (*total_remaining_blocks != 0) {
		si_buffer = zalloc(fs->blksz);
		if (!si_buffer) {
			printf("No Memory\n");
			return;
		}
		si_start_addr = si_buffer;
		si_blockno = ext4fs_get_new_blk_no();
		if (si_blockno == -1) {
			printf("no block left to assign\n");
			goto fail;
		}
		(*no_blks_reqd)++;
		debug("SIPB %ld: %u\n", si_blockno, *total_remaining_blocks);

		status = ext4fs_devread((lbaint_t)si_blockno * fs->sect_perblk,
					0, fs->blksz, (char *)si_buffer);
		memset(si_buffer, '\0', fs->blksz);
		if (status == 0)
			goto fail;

		for (i = 0; i < (fs->blksz / sizeof(int)); i++) {
			actual_block_no = ext4fs_get_new_blk_no();
			if (actual_block_no == -1) {
				printf("no block left to assign\n");
				goto fail;
			}
			*si_buffer = actual_block_no;
			debug("SIAB %u: %u\n", *si_buffer,
				*total_remaining_blocks);

			si_buffer++;
			(*total_remaining_blocks)--;
			if (*total_remaining_blocks == 0)
				break;
		}

		/* write the block to disk */
		put_ext4(((uint64_t) ((uint64_t)si_blockno * (uint64_t)fs->blksz)),
			 si_start_addr, fs->blksz);
		file_inode->b.blocks.indir_block = si_blockno;
	}
fail:
	free(si_start_addr);
}

static void alloc_double_indirect_block(struct ext2_inode *file_inode,
					unsigned int *total_remaining_blocks,
					unsigned int *no_blks_reqd)
{
	short i;
	short j;
	short status;
	long int actual_block_no;
	/* di:double indirect */
	long int di_blockno_parent;
	long int di_blockno_child;
	unsigned int *di_parent_buffer = NULL;
	unsigned int *di_child_buff = NULL;
	unsigned int *di_block_start_addr = NULL;
	unsigned int *di_child_buff_start = NULL;
	struct ext_filesystem *fs = get_fs();

	if (*total_remaining_blocks != 0) {
		/* double indirect parent block connecting to inode */
		di_blockno_parent = ext4fs_get_new_blk_no();
		if (di_blockno_parent == -1) {
			printf("no block left to assign\n");
			goto fail;
		}
		di_parent_buffer = zalloc(fs->blksz);
		if (!di_parent_buffer)
			goto fail;

		di_block_start_addr = di_parent_buffer;
		(*no_blks_reqd)++;
		debug("DIPB %ld: %u\n", di_blockno_parent,
		      *total_remaining_blocks);

		status = ext4fs_devread((lbaint_t)di_blockno_parent *
					fs->sect_perblk, 0,
					fs->blksz, (char *)di_parent_buffer);

		if (!status) {
			printf("%s: Device read error!\n", __func__);
			goto fail;
		}
		memset(di_parent_buffer, '\0', fs->blksz);

		/*
		 * start:for each double indirect parent
		 * block create one more block
		 */
		for (i = 0; i < (fs->blksz / sizeof(int)); i++) {
			di_blockno_child = ext4fs_get_new_blk_no();
			if (di_blockno_child == -1) {
				printf("no block left to assign\n");
				goto fail;
			}
			di_child_buff = zalloc(fs->blksz);
			if (!di_child_buff)
				goto fail;

			di_child_buff_start = di_child_buff;
			*di_parent_buffer = di_blockno_child;
			di_parent_buffer++;
			(*no_blks_reqd)++;
			debug("DICB %ld: %u\n", di_blockno_child,
			      *total_remaining_blocks);

			status = ext4fs_devread((lbaint_t)di_blockno_child *
						fs->sect_perblk, 0,
						fs->blksz,
						(char *)di_child_buff);

			if (!status) {
				printf("%s: Device read error!\n", __func__);
				goto fail;
			}
			memset(di_child_buff, '\0', fs->blksz);
			/* filling of actual datablocks for each child */
			for (j = 0; j < (fs->blksz / sizeof(int)); j++) {
				actual_block_no = ext4fs_get_new_blk_no();
				if (actual_block_no == -1) {
					printf("no block left to assign\n");
					goto fail;
				}
				*di_child_buff = actual_block_no;
				debug("DIAB %ld: %u\n", actual_block_no,
				      *total_remaining_blocks);

				di_child_buff++;
				(*total_remaining_blocks)--;
				if (*total_remaining_blocks == 0)
					break;
			}
			/* write the block  table */
			put_ext4(((uint64_t) ((uint64_t)di_blockno_child * (uint64_t)fs->blksz)),
				 di_child_buff_start, fs->blksz);
			free(di_child_buff_start);
			di_child_buff_start = NULL;

			if (*total_remaining_blocks == 0)
				break;
		}
		put_ext4(((uint64_t) ((uint64_t)di_blockno_parent * (uint64_t)fs->blksz)),
			 di_block_start_addr, fs->blksz);
		file_inode->b.blocks.double_indir_block = di_blockno_parent;
	}
fail:
	free(di_block_start_addr);
}

static void alloc_triple_indirect_block(struct ext2_inode *file_inode,
					unsigned int *total_remaining_blocks,
					unsigned int *no_blks_reqd)
{
	short i;
	short j;
	short k;
	long int actual_block_no;
	/* ti: Triple Indirect */
	long int ti_gp_blockno;
	long int ti_parent_blockno;
	long int ti_child_blockno;
	unsigned int *ti_gp_buff = NULL;
	unsigned int *ti_parent_buff = NULL;
	unsigned int *ti_child_buff = NULL;
	unsigned int *ti_gp_buff_start_addr = NULL;
	unsigned int *ti_pbuff_start_addr = NULL;
	unsigned int *ti_cbuff_start_addr = NULL;
	struct ext_filesystem *fs = get_fs();
	if (*total_remaining_blocks != 0) {
		/* triple indirect grand parent block connecting to inode */
		ti_gp_blockno = ext4fs_get_new_blk_no();
		if (ti_gp_blockno == -1) {
			printf("no block left to assign\n");
			return;
		}
		ti_gp_buff = zalloc(fs->blksz);
		if (!ti_gp_buff)
			return;

		ti_gp_buff_start_addr = ti_gp_buff;
		(*no_blks_reqd)++;
		debug("TIGPB %ld: %u\n", ti_gp_blockno,
		      *total_remaining_blocks);

		/* for each 4 byte grand parent entry create one more block */
		for (i = 0; i < (fs->blksz / sizeof(int)); i++) {
			ti_parent_blockno = ext4fs_get_new_blk_no();
			if (ti_parent_blockno == -1) {
				printf("no block left to assign\n");
				goto fail;
			}
			ti_parent_buff = zalloc(fs->blksz);
			if (!ti_parent_buff)
				goto fail;

			ti_pbuff_start_addr = ti_parent_buff;
			*ti_gp_buff = ti_parent_blockno;
			ti_gp_buff++;
			(*no_blks_reqd)++;
			debug("TIPB %ld: %u\n", ti_parent_blockno,
			      *total_remaining_blocks);

			/* for each 4 byte entry parent create one more block */
			for (j = 0; j < (fs->blksz / sizeof(int)); j++) {
				ti_child_blockno = ext4fs_get_new_blk_no();
				if (ti_child_blockno == -1) {
					printf("no block left assign\n");
					goto fail1;
				}
				ti_child_buff = zalloc(fs->blksz);
				if (!ti_child_buff)
					goto fail1;

				ti_cbuff_start_addr = ti_child_buff;
				*ti_parent_buff = ti_child_blockno;
				ti_parent_buff++;
				(*no_blks_reqd)++;
				debug("TICB %ld: %u\n", ti_parent_blockno,
				      *total_remaining_blocks);

				/* fill actual datablocks for each child */
				for (k = 0; k < (fs->blksz / sizeof(int));
					k++) {
					actual_block_no =
					    ext4fs_get_new_blk_no();
					if (actual_block_no == -1) {
						printf("no block left\n");
						free(ti_cbuff_start_addr);
						goto fail1;
					}
					*ti_child_buff = actual_block_no;
					debug("TIAB %ld: %u\n", actual_block_no,
					      *total_remaining_blocks);

					ti_child_buff++;
					(*total_remaining_blocks)--;
					if (*total_remaining_blocks == 0)
						break;
				}
				/* write the child block */
				put_ext4(((uint64_t) ((uint64_t)ti_child_blockno *
						      (uint64_t)fs->blksz)),
					 ti_cbuff_start_addr, fs->blksz);
				free(ti_cbuff_start_addr);

				if (*total_remaining_blocks == 0)
					break;
			}
			/* write the parent block */
			put_ext4(((uint64_t) ((uint64_t)ti_parent_blockno * (uint64_t)fs->blksz)),
				 ti_pbuff_start_addr, fs->blksz);
			free(ti_pbuff_start_addr);

			if (*total_remaining_blocks == 0)
				break;
		}
		/* write the grand parent block */
		put_ext4(((uint64_t) ((uint64_t)ti_gp_blockno * (uint64_t)fs->blksz)),
			 ti_gp_buff_start_addr, fs->blksz);
		file_inode->b.blocks.triple_indir_block = ti_gp_blockno;
		free(ti_gp_buff_start_addr);
		return;
	}
fail1:
	free(ti_pbuff_start_addr);
fail:
	free(ti_gp_buff_start_addr);
}

void ext4fs_allocate_blocks(struct ext2_inode *file_inode,
				unsigned int total_remaining_blocks,
				unsigned int *total_no_of_block)
{
	short i;
	long int direct_blockno;
	unsigned int no_blks_reqd = 0;

	/* allocation of direct blocks */
	for (i = 0; total_remaining_blocks && i < INDIRECT_BLOCKS; i++) {
		direct_blockno = ext4fs_get_new_blk_no();
		if (direct_blockno == -1) {
			printf("no block left to assign\n");
			return;
		}
		file_inode->b.blocks.dir_blocks[i] = direct_blockno;
		debug("DB %ld: %u\n", direct_blockno, total_remaining_blocks);

		total_remaining_blocks--;
	}

	alloc_single_indirect_block(file_inode, &total_remaining_blocks,
				    &no_blks_reqd);
	alloc_double_indirect_block(file_inode, &total_remaining_blocks,
				    &no_blks_reqd);
	alloc_triple_indirect_block(file_inode, &total_remaining_blocks,
				    &no_blks_reqd);
	*total_no_of_block += no_blks_reqd;
}

#endif

static struct ext4_extent_header *ext4fs_get_extent_block
	(struct ext2_data *data, char *buf,
		struct ext4_extent_header *ext_block,
		uint32_t fileblock, int log2_blksz)
{
	struct ext4_extent_idx *index;
	unsigned long long block;
	int blksz = EXT2_BLOCK_SIZE(data);
	int i;

	while (1) {
		index = (struct ext4_extent_idx *)(ext_block + 1);

		if (le16_to_cpu(ext_block->eh_magic) != EXT4_EXT_MAGIC)
			return 0;

		if (ext_block->eh_depth == 0)
			return ext_block;
		i = -1;
		do {
			i++;
			if (i >= le16_to_cpu(ext_block->eh_entries))
				break;
		} while (fileblock >= le32_to_cpu(index[i].ei_block));

		if (--i < 0)
			return 0;

		block = le16_to_cpu(index[i].ei_leaf_hi);
		block = (block << 32) + le32_to_cpu(index[i].ei_leaf_lo);

		if (ext4fs_devread((lbaint_t)block << log2_blksz, 0, blksz,
				   buf))
			ext_block = (struct ext4_extent_header *)buf;
		else
			return 0;
	}
}

static int ext4fs_blockgroup
	(struct ext2_data *data, int group, struct ext2_block_group *blkgrp)
{
	long int blkno;
	unsigned int blkoff, desc_per_blk;
	int log2blksz = get_fs()->dev_desc->log2blksz;

	desc_per_blk = EXT2_BLOCK_SIZE(data) / sizeof(struct ext2_block_group);

	blkno = __le32_to_cpu(data->sblock.first_data_block) + 1 +
			group / desc_per_blk;
	blkoff = (group % desc_per_blk) * sizeof(struct ext2_block_group);

	debug("ext4fs read %d group descriptor (blkno %ld blkoff %u)\n",
	      group, blkno, blkoff);

	return ext4fs_devread((lbaint_t)blkno <<
			      (LOG2_BLOCK_SIZE(data) - log2blksz),
			      blkoff, sizeof(struct ext2_block_group),
			      (char *)blkgrp);
}

int ext4fs_read_inode(struct ext2_data *data, int ino, struct ext2_inode *inode)
{
	struct ext2_block_group blkgrp;
	struct ext2_sblock *sblock = &data->sblock;
	struct ext_filesystem *fs = get_fs();
	int log2blksz = get_fs()->dev_desc->log2blksz;
	int inodes_per_block, status;
	long int blkno;
	unsigned int blkoff;

	/* It is easier to calculate if the first inode is 0. */
	ino--;
	status = ext4fs_blockgroup(data, ino / __le32_to_cpu
				   (sblock->inodes_per_group), &blkgrp);
	if (status == 0)
		return 0;

	inodes_per_block = EXT2_BLOCK_SIZE(data) / fs->inodesz;
	blkno = __le32_to_cpu(blkgrp.inode_table_id) +
	    (ino % __le32_to_cpu(sblock->inodes_per_group)) / inodes_per_block;
	blkoff = (ino % inodes_per_block) * fs->inodesz;
	/* Read the inode. */
	status = ext4fs_devread((lbaint_t)blkno << (LOG2_BLOCK_SIZE(data) -
				log2blksz), blkoff,
				sizeof(struct ext2_inode), (char *)inode);
	if (status == 0)
		return 0;

	return 1;
}

long int read_allocated_block(struct ext2_inode *inode, int fileblock)
{
	long int blknr;
	int blksz;
	int log2_blksz;
	int status;
	long int rblock;
	long int perblock_parent;
	long int perblock_child;
	unsigned long long start;
	/* get the blocksize of the filesystem */
	blksz = EXT2_BLOCK_SIZE(ext4fs_root);
	log2_blksz = LOG2_BLOCK_SIZE(ext4fs_root)
		- get_fs()->dev_desc->log2blksz;

	if (le32_to_cpu(inode->flags) & EXT4_EXTENTS_FL) {
		char *buf = zalloc(blksz);
		if (!buf)
			return -ENOMEM;
		struct ext4_extent_header *ext_block;
		struct ext4_extent *extent;
		int i = -1;
		ext_block =
			ext4fs_get_extent_block(ext4fs_root, buf,
						(struct ext4_extent_header *)
						inode->b.blocks.dir_blocks,
						fileblock, log2_blksz);
		if (!ext_block) {
			printf("invalid extent block\n");
			free(buf);
			return -EINVAL;
		}

		extent = (struct ext4_extent *)(ext_block + 1);

		do {
			i++;
			if (i >= le16_to_cpu(ext_block->eh_entries))
				break;
		} while (fileblock >= le32_to_cpu(extent[i].ee_block));
		if (--i >= 0) {
			fileblock -= le32_to_cpu(extent[i].ee_block);
			if (fileblock >= le16_to_cpu(extent[i].ee_len)) {
				free(buf);
				return 0;
			}

			start = le16_to_cpu(extent[i].ee_start_hi);
			start = (start << 32) +
					le32_to_cpu(extent[i].ee_start_lo);
			free(buf);
			return fileblock + start;
		}

		printf("Extent Error\n");
		free(buf);
		return -1;
	}

	/* Direct blocks. */
	if (fileblock < INDIRECT_BLOCKS)
		blknr = __le32_to_cpu(inode->b.blocks.dir_blocks[fileblock]);

	/* Indirect. */
	else if (fileblock < (INDIRECT_BLOCKS + (blksz / 4))) {
		if (ext4fs_indir1_block == NULL) {
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** SI ext2fs read block (indir 1)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
			ext4fs_indir1_blkno = -1;
		}
		if (blksz != ext4fs_indir1_size) {
			free(ext4fs_indir1_block);
			ext4fs_indir1_block = NULL;
			ext4fs_indir1_size = 0;
			ext4fs_indir1_blkno = -1;
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** SI ext2fs read block (indir 1):"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
		}
		if ((__le32_to_cpu(inode->b.blocks.indir_block) <<
		     log2_blksz) != ext4fs_indir1_blkno) {
			status =
			    ext4fs_devread((lbaint_t)__le32_to_cpu
					   (inode->b.blocks.
					    indir_block) << log2_blksz, 0,
					   blksz, (char *)ext4fs_indir1_block);
			if (status == 0) {
				printf("** SI ext2fs read block (indir 1)"
					"failed. **\n");
				return 0;
			}
			ext4fs_indir1_blkno =
				__le32_to_cpu(inode->b.blocks.
					       indir_block) << log2_blksz;
		}
		blknr = __le32_to_cpu(ext4fs_indir1_block
				      [fileblock - INDIRECT_BLOCKS]);
	}
	/* Double indirect. */
	else if (fileblock < (INDIRECT_BLOCKS + (blksz / 4 *
					(blksz / 4 + 1)))) {

		long int perblock = blksz / 4;
		long int rblock = fileblock - (INDIRECT_BLOCKS + blksz / 4);

		if (ext4fs_indir1_block == NULL) {
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** DI ext2fs read block (indir 2 1)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
			ext4fs_indir1_blkno = -1;
		}
		if (blksz != ext4fs_indir1_size) {
			free(ext4fs_indir1_block);
			ext4fs_indir1_block = NULL;
			ext4fs_indir1_size = 0;
			ext4fs_indir1_blkno = -1;
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** DI ext2fs read block (indir 2 1)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
		}
		if ((__le32_to_cpu(inode->b.blocks.double_indir_block) <<
		     log2_blksz) != ext4fs_indir1_blkno) {
			status =
			    ext4fs_devread((lbaint_t)__le32_to_cpu
					   (inode->b.blocks.
					    double_indir_block) << log2_blksz,
					   0, blksz,
					   (char *)ext4fs_indir1_block);
			if (status == 0) {
				printf("** DI ext2fs read block (indir 2 1)"
					"failed. **\n");
				return -1;
			}
			ext4fs_indir1_blkno =
			    __le32_to_cpu(inode->b.blocks.double_indir_block) <<
			    log2_blksz;
		}

		if (ext4fs_indir2_block == NULL) {
			ext4fs_indir2_block = zalloc(blksz);
			if (ext4fs_indir2_block == NULL) {
				printf("** DI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir2_size = blksz;
			ext4fs_indir2_blkno = -1;
		}
		if (blksz != ext4fs_indir2_size) {
			free(ext4fs_indir2_block);
			ext4fs_indir2_block = NULL;
			ext4fs_indir2_size = 0;
			ext4fs_indir2_blkno = -1;
			ext4fs_indir2_block = zalloc(blksz);
			if (ext4fs_indir2_block == NULL) {
				printf("** DI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir2_size = blksz;
		}
		if ((__le32_to_cpu(ext4fs_indir1_block[rblock / perblock]) <<
		     log2_blksz) != ext4fs_indir2_blkno) {
			status = ext4fs_devread((lbaint_t)__le32_to_cpu
						(ext4fs_indir1_block
						 [rblock /
						  perblock]) << log2_blksz, 0,
						blksz,
						(char *)ext4fs_indir2_block);
			if (status == 0) {
				printf("** DI ext2fs read block (indir 2 2)"
					"failed. **\n");
				return -1;
			}
			ext4fs_indir2_blkno =
			    __le32_to_cpu(ext4fs_indir1_block[rblock
							      /
							      perblock]) <<
			    log2_blksz;
		}
		blknr = __le32_to_cpu(ext4fs_indir2_block[rblock % perblock]);
	}
	/* Tripple indirect. */
	else {
		rblock = fileblock - (INDIRECT_BLOCKS + blksz / 4 +
				      (blksz / 4 * blksz / 4));
		perblock_child = blksz / 4;
		perblock_parent = ((blksz / 4) * (blksz / 4));

		if (ext4fs_indir1_block == NULL) {
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** TI ext2fs read block (indir 2 1)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
			ext4fs_indir1_blkno = -1;
		}
		if (blksz != ext4fs_indir1_size) {
			free(ext4fs_indir1_block);
			ext4fs_indir1_block = NULL;
			ext4fs_indir1_size = 0;
			ext4fs_indir1_blkno = -1;
			ext4fs_indir1_block = zalloc(blksz);
			if (ext4fs_indir1_block == NULL) {
				printf("** TI ext2fs read block (indir 2 1)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir1_size = blksz;
		}
		if ((__le32_to_cpu(inode->b.blocks.triple_indir_block) <<
		     log2_blksz) != ext4fs_indir1_blkno) {
			status = ext4fs_devread
			    ((lbaint_t)
			     __le32_to_cpu(inode->b.blocks.triple_indir_block)
			     << log2_blksz, 0, blksz,
			     (char *)ext4fs_indir1_block);
			if (status == 0) {
				printf("** TI ext2fs read block (indir 2 1)"
					"failed. **\n");
				return -1;
			}
			ext4fs_indir1_blkno =
			    __le32_to_cpu(inode->b.blocks.triple_indir_block) <<
			    log2_blksz;
		}

		if (ext4fs_indir2_block == NULL) {
			ext4fs_indir2_block = zalloc(blksz);
			if (ext4fs_indir2_block == NULL) {
				printf("** TI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir2_size = blksz;
			ext4fs_indir2_blkno = -1;
		}
		if (blksz != ext4fs_indir2_size) {
			free(ext4fs_indir2_block);
			ext4fs_indir2_block = NULL;
			ext4fs_indir2_size = 0;
			ext4fs_indir2_blkno = -1;
			ext4fs_indir2_block = zalloc(blksz);
			if (ext4fs_indir2_block == NULL) {
				printf("** TI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir2_size = blksz;
		}
		if ((__le32_to_cpu(ext4fs_indir1_block[rblock /
						       perblock_parent]) <<
		     log2_blksz)
		    != ext4fs_indir2_blkno) {
			status = ext4fs_devread((lbaint_t)__le32_to_cpu
						(ext4fs_indir1_block
						 [rblock /
						  perblock_parent]) <<
						log2_blksz, 0, blksz,
						(char *)ext4fs_indir2_block);
			if (status == 0) {
				printf("** TI ext2fs read block (indir 2 2)"
					"failed. **\n");
				return -1;
			}
			ext4fs_indir2_blkno =
			    __le32_to_cpu(ext4fs_indir1_block[rblock /
							      perblock_parent])
			    << log2_blksz;
		}

		if (ext4fs_indir3_block == NULL) {
			ext4fs_indir3_block = zalloc(blksz);
			if (ext4fs_indir3_block == NULL) {
				printf("** TI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir3_size = blksz;
			ext4fs_indir3_blkno = -1;
		}
		if (blksz != ext4fs_indir3_size) {
			free(ext4fs_indir3_block);
			ext4fs_indir3_block = NULL;
			ext4fs_indir3_size = 0;
			ext4fs_indir3_blkno = -1;
			ext4fs_indir3_block = zalloc(blksz);
			if (ext4fs_indir3_block == NULL) {
				printf("** TI ext2fs read block (indir 2 2)"
					"malloc failed. **\n");
				return -1;
			}
			ext4fs_indir3_size = blksz;
		}
		if ((__le32_to_cpu(ext4fs_indir2_block[rblock
						       /
						       perblock_child]) <<
		     log2_blksz) != ext4fs_indir3_blkno) {
			status =
			    ext4fs_devread((lbaint_t)__le32_to_cpu
					   (ext4fs_indir2_block
					    [(rblock / perblock_child)
					     % (blksz / 4)]) << log2_blksz, 0,
					   blksz, (char *)ext4fs_indir3_block);
			if (status == 0) {
				printf("** TI ext2fs read block (indir 2 2)"
				       "failed. **\n");
				return -1;
			}
			ext4fs_indir3_blkno =
			    __le32_to_cpu(ext4fs_indir2_block[(rblock /
							       perblock_child) %
							      (blksz /
							       4)]) <<
			    log2_blksz;
		}

		blknr = __le32_to_cpu(ext4fs_indir3_block
				      [rblock % perblock_child]);
	}
	debug("read_allocated_block %ld\n", blknr);

	return blknr;
}

/**
 * ext4fs_reinit_global() - Reinitialize values of ext4 write implementation's
 *			    global pointers
 *
 * This function assures that for a file with the same name but different size
 * the sequential store on the ext4 filesystem will be correct.
 *
 * In this function the global data, responsible for internal representation
 * of the ext4 data are initialized to the reset state. Without this, during
 * replacement of the smaller file with the bigger truncation of new file was
 * performed.
 */
void ext4fs_reinit_global(void)
{
	if (ext4fs_indir1_block != NULL) {
		free(ext4fs_indir1_block);
		ext4fs_indir1_block = NULL;
		ext4fs_indir1_size = 0;
		ext4fs_indir1_blkno = -1;
	}
	if (ext4fs_indir2_block != NULL) {
		free(ext4fs_indir2_block);
		ext4fs_indir2_block = NULL;
		ext4fs_indir2_size = 0;
		ext4fs_indir2_blkno = -1;
	}
	if (ext4fs_indir3_block != NULL) {
		free(ext4fs_indir3_block);
		ext4fs_indir3_block = NULL;
		ext4fs_indir3_size = 0;
		ext4fs_indir3_blkno = -1;
	}
}
void ext4fs_close(void)
{
	if ((ext4fs_file != NULL) && (ext4fs_root != NULL)) {
		ext4fs_free_node(ext4fs_file, &ext4fs_root->diropen);
		ext4fs_file = NULL;
	}
	if (ext4fs_root != NULL) {
		free(ext4fs_root);
		ext4fs_root = NULL;
	}

	ext4fs_reinit_global();
}

int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name,
				struct ext2fs_node **fnode, int *ftype)
{
	unsigned int fpos = 0;
	int status;
	loff_t actread;
	struct ext2fs_node *diro = (struct ext2fs_node *) dir;

#ifdef DEBUG
	if (name != NULL)
		printf("Iterate dir %s\n", name);
#endif /* of DEBUG */
	if (!diro->inode_read) {
		status = ext4fs_read_inode(diro->data, diro->ino, &diro->inode);
		if (status == 0)
			return 0;
	}
	/* Search the file.  */
	while (fpos < __le32_to_cpu(diro->inode.size)) {
		struct ext2_dirent dirent;

		status = ext4fs_read_file(diro, fpos,
					   sizeof(struct ext2_dirent),
					   (char *)&dirent, &actread);
		if (status < 0)
			return 0;

		if (dirent.direntlen == 0) {
			printf("Failed to iterate over directory %s\n", name);
			return 0;
		}

		if (dirent.namelen != 0) {
			char filename[dirent.namelen + 1];
			struct ext2fs_node *fdiro;
			int type = FILETYPE_UNKNOWN;

			status = ext4fs_read_file(diro,
						  fpos +
						  sizeof(struct ext2_dirent),
						  dirent.namelen, filename,
						  &actread);
			if (status < 0)
				return 0;

			fdiro = zalloc(sizeof(struct ext2fs_node));
			if (!fdiro)
				return 0;

			fdiro->data = diro->data;
			fdiro->ino = __le32_to_cpu(dirent.inode);

			filename[dirent.namelen] = '\0';

			if (dirent.filetype != FILETYPE_UNKNOWN) {
				fdiro->inode_read = 0;

				if (dirent.filetype == FILETYPE_DIRECTORY)
					type = FILETYPE_DIRECTORY;
				else if (dirent.filetype == FILETYPE_SYMLINK)
					type = FILETYPE_SYMLINK;
				else if (dirent.filetype == FILETYPE_REG)
					type = FILETYPE_REG;
			} else {
				status = ext4fs_read_inode(diro->data,
							   __le32_to_cpu
							   (dirent.inode),
							   &fdiro->inode);
				if (status == 0) {
					free(fdiro);
					return 0;
				}
				fdiro->inode_read = 1;

				if ((__le16_to_cpu(fdiro->inode.mode) &
				     FILETYPE_INO_MASK) ==
				    FILETYPE_INO_DIRECTORY) {
					type = FILETYPE_DIRECTORY;
				} else if ((__le16_to_cpu(fdiro->inode.mode)
					    & FILETYPE_INO_MASK) ==
					   FILETYPE_INO_SYMLINK) {
					type = FILETYPE_SYMLINK;
				} else if ((__le16_to_cpu(fdiro->inode.mode)
					    & FILETYPE_INO_MASK) ==
					   FILETYPE_INO_REG) {
					type = FILETYPE_REG;
				}
			}
#ifdef DEBUG
			printf("iterate >%s<\n", filename);
#endif /* of DEBUG */
			if ((name != NULL) && (fnode != NULL)
			    && (ftype != NULL)) {
				if (strcmp(filename, name) == 0) {
					*ftype = type;
					*fnode = fdiro;
					return 1;
				}
			} else {
				if (fdiro->inode_read == 0) {
					status = ext4fs_read_inode(diro->data,
								 __le32_to_cpu(
								 dirent.inode),
								 &fdiro->inode);
					if (status == 0) {
						free(fdiro);
						return 0;
					}
					fdiro->inode_read = 1;
				}
				switch (type) {
				case FILETYPE_DIRECTORY:
					printf("<DIR> ");
					break;
				case FILETYPE_SYMLINK:
					printf("<SYM> ");
					break;
				case FILETYPE_REG:
					printf("      ");
					break;
				default:
					printf("< ? > ");
					break;
				}
				printf("%10u %s\n",
				       __le32_to_cpu(fdiro->inode.size),
					filename);
			}
			free(fdiro);
		}
		fpos += __le16_to_cpu(dirent.direntlen);
	}
	return 0;
}

static char *ext4fs_read_symlink(struct ext2fs_node *node)
{
	char *symlink;
	struct ext2fs_node *diro = node;
	int status;
	loff_t actread;

	if (!diro->inode_read) {
		status = ext4fs_read_inode(diro->data, diro->ino, &diro->inode);
		if (status == 0)
			return 0;
	}
	symlink = zalloc(__le32_to_cpu(diro->inode.size) + 1);
	if (!symlink)
		return 0;

	if (__le32_to_cpu(diro->inode.size) < sizeof(diro->inode.b.symlink)) {
		strncpy(symlink, diro->inode.b.symlink,
			 __le32_to_cpu(diro->inode.size));
	} else {
		status = ext4fs_read_file(diro, 0,
					   __le32_to_cpu(diro->inode.size),
					   symlink, &actread);
		if ((status < 0) || (actread == 0)) {
			free(symlink);
			return 0;
		}
	}
	symlink[__le32_to_cpu(diro->inode.size)] = '\0';
	return symlink;
}

static int ext4fs_find_file1(const char *currpath,
			     struct ext2fs_node *currroot,
			     struct ext2fs_node **currfound, int *foundtype)
{
	char fpath[strlen(currpath) + 1];
	char *name = fpath;
	char *next;
	int status;
	int type = FILETYPE_DIRECTORY;
	struct ext2fs_node *currnode = currroot;
	struct ext2fs_node *oldnode = currroot;

	strncpy(fpath, currpath, strlen(currpath) + 1);

	/* Remove all leading slashes. */
	while (*name == '/')
		name++;

	if (!*name) {
		*currfound = currnode;
		return 1;
	}

	for (;;) {
		int found;

		/* Extract the actual part from the pathname. */
		next = strchr(name, '/');
		if (next) {
			/* Remove all leading slashes. */
			while (*next == '/')
				*(next++) = '\0';
		}

		if (type != FILETYPE_DIRECTORY) {
			ext4fs_free_node(currnode, currroot);
			return 0;
		}

		oldnode = currnode;

		/* Iterate over the directory. */
		found = ext4fs_iterate_dir(currnode, name, &currnode, &type);
		if (found == 0)
			return 0;

		if (found == -1)
			break;

		/* Read in the symlink and follow it. */
		if (type == FILETYPE_SYMLINK) {
			char *symlink;

			/* Test if the symlink does not loop. */
			if (++symlinknest == 8) {
				ext4fs_free_node(currnode, currroot);
				ext4fs_free_node(oldnode, currroot);
				return 0;
			}

			symlink = ext4fs_read_symlink(currnode);
			ext4fs_free_node(currnode, currroot);

			if (!symlink) {
				ext4fs_free_node(oldnode, currroot);
				return 0;
			}

			debug("Got symlink >%s<\n", symlink);

			if (symlink[0] == '/') {
				ext4fs_free_node(oldnode, currroot);
				oldnode = &ext4fs_root->diropen;
			}

			/* Lookup the node the symlink points to. */
			status = ext4fs_find_file1(symlink, oldnode,
						    &currnode, &type);

			free(symlink);

			if (status == 0) {
				ext4fs_free_node(oldnode, currroot);
				return 0;
			}
		}

		ext4fs_free_node(oldnode, currroot);

		/* Found the node! */
		if (!next || *next == '\0') {
			*currfound = currnode;
			*foundtype = type;
			return 1;
		}
		name = next;
	}
	return -1;
}

int ext4fs_find_file(const char *path, struct ext2fs_node *rootnode,
	struct ext2fs_node **foundnode, int expecttype)
{
	int status;
	int foundtype = FILETYPE_DIRECTORY;

	symlinknest = 0;
	if (!path)
		return 0;

	status = ext4fs_find_file1(path, rootnode, foundnode, &foundtype);
	if (status == 0)
		return 0;

	/* Check if the node that was found was of the expected type. */
	if ((expecttype == FILETYPE_REG) && (foundtype != expecttype))
		return 0;
	else if ((expecttype == FILETYPE_DIRECTORY)
		   && (foundtype != expecttype))
		return 0;

	return 1;
}

int ext4fs_open(const char *filename, loff_t *len)
{
	struct ext2fs_node *fdiro = NULL;
	int status;

	if (ext4fs_root == NULL)
		return -1;

	ext4fs_file = NULL;
	status = ext4fs_find_file(filename, &ext4fs_root->diropen, &fdiro,
				  FILETYPE_REG);
	if (status == 0)
		goto fail;

	if (!fdiro->inode_read) {
		status = ext4fs_read_inode(fdiro->data, fdiro->ino,
				&fdiro->inode);
		if (status == 0)
			goto fail;
	}
	*len = __le32_to_cpu(fdiro->inode.size);
	ext4fs_file = fdiro;

	return 0;
fail:
	ext4fs_free_node(fdiro, &ext4fs_root->diropen);

	return -1;
}

int ext4fs_mount(unsigned part_length)
{
	struct ext2_data *data;
	int status;
	struct ext_filesystem *fs = get_fs();
	data = zalloc(SUPERBLOCK_SIZE);
	if (!data)
		return 0;

	/* Read the superblock. */
	status = ext4_read_superblock((char *)&data->sblock);

	if (status == 0)
		goto fail;

	/* Make sure this is an ext2 filesystem. */
	if (__le16_to_cpu(data->sblock.magic) != EXT2_MAGIC)
		goto fail;

	if (__le32_to_cpu(data->sblock.revision_level == 0))
		fs->inodesz = 128;
	else
		fs->inodesz = __le16_to_cpu(data->sblock.inode_size);

	debug("EXT2 rev %d, inode_size %d\n",
	       __le32_to_cpu(data->sblock.revision_level), fs->inodesz);

	data->diropen.data = data;
	data->diropen.ino = 2;
	data->diropen.inode_read = 1;
	data->inode = &data->diropen.inode;

	status = ext4fs_read_inode(data, 2, data->inode);
	if (status == 0)
		goto fail;

	ext4fs_root = data;

	return 1;
fail:
	printf("Failed to mount ext2 filesystem...\n");
	free(data);
	ext4fs_root = NULL;

	return 0;
}
