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

#include <common.h>
#include <ext_common.h>
#include <ext4fs.h>
#include <malloc.h>
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

static struct ext4_extent_header *ext4fs_get_extent_block
	(struct ext2_data *data, char *buf,
		struct ext4_extent_header *ext_block,
		uint32_t fileblock, int log2_blksz)
{
	struct ext4_extent_idx *index;
	unsigned long long block;
	struct ext_filesystem *fs = get_fs();
	int i;

	while (1) {
		index = (struct ext4_extent_idx *)(ext_block + 1);

		if (le32_to_cpu(ext_block->eh_magic) != EXT4_EXT_MAGIC)
			return 0;

		if (ext_block->eh_depth == 0)
			return ext_block;
		i = -1;
		do {
			i++;
			if (i >= le32_to_cpu(ext_block->eh_entries))
				break;
		} while (fileblock > le32_to_cpu(index[i].ei_block));

		if (--i < 0)
			return 0;

		block = le32_to_cpu(index[i].ei_leaf_hi);
		block = (block << 32) + le32_to_cpu(index[i].ei_leaf_lo);

		if (ext4fs_devread(block << log2_blksz, 0, fs->blksz, buf))
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

	desc_per_blk = EXT2_BLOCK_SIZE(data) / sizeof(struct ext2_block_group);

	blkno = __le32_to_cpu(data->sblock.first_data_block) + 1 +
			group / desc_per_blk;
	blkoff = (group % desc_per_blk) * sizeof(struct ext2_block_group);

	debug("ext4fs read %d group descriptor (blkno %ld blkoff %u)\n",
	      group, blkno, blkoff);

	return ext4fs_devread(blkno << LOG2_EXT2_BLOCK_SIZE(data),
			      blkoff, sizeof(struct ext2_block_group),
			      (char *)blkgrp);
}

int ext4fs_read_inode(struct ext2_data *data, int ino, struct ext2_inode *inode)
{
	struct ext2_block_group blkgrp;
	struct ext2_sblock *sblock = &data->sblock;
	struct ext_filesystem *fs = get_fs();
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
	status = ext4fs_devread(blkno << LOG2_EXT2_BLOCK_SIZE(data), blkoff,
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
	log2_blksz = LOG2_EXT2_BLOCK_SIZE(ext4fs_root);
	if (le32_to_cpu(inode->flags) & EXT4_EXTENTS_FL) {
		char *buf = zalloc(blksz);
		if (!buf)
			return -ENOMEM;
		struct ext4_extent_header *ext_block;
		struct ext4_extent *extent;
		int i = -1;
		ext_block = ext4fs_get_extent_block(ext4fs_root, buf,
						    (struct ext4_extent_header
						     *)inode->b.
						    blocks.dir_blocks,
						    fileblock, log2_blksz);
		if (!ext_block) {
			printf("invalid extent block\n");
			free(buf);
			return -EINVAL;
		}

		extent = (struct ext4_extent *)(ext_block + 1);

		do {
			i++;
			if (i >= le32_to_cpu(ext_block->eh_entries))
				break;
		} while (fileblock >= le32_to_cpu(extent[i].ee_block));
		if (--i >= 0) {
			fileblock -= le32_to_cpu(extent[i].ee_block);
			if (fileblock >= le32_to_cpu(extent[i].ee_len)) {
				free(buf);
				return 0;
			}

			start = le32_to_cpu(extent[i].ee_start_hi);
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
			    ext4fs_devread(__le32_to_cpu
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
			    ext4fs_devread(__le32_to_cpu
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
			status = ext4fs_devread(__le32_to_cpu
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
			    (__le32_to_cpu(inode->b.blocks.triple_indir_block)
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
			status = ext4fs_devread(__le32_to_cpu
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
			    ext4fs_devread(__le32_to_cpu
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
	debug("ext4fs_read_block %ld\n", blknr);

	return blknr;
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

int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name,
				struct ext2fs_node **fnode, int *ftype)
{
	unsigned int fpos = 0;
	int status;
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
					   (char *) &dirent);
		if (status < 1)
			return 0;

		if (dirent.namelen != 0) {
			char filename[dirent.namelen + 1];
			struct ext2fs_node *fdiro;
			int type = FILETYPE_UNKNOWN;

			status = ext4fs_read_file(diro,
						  fpos +
						  sizeof(struct ext2_dirent),
						  dirent.namelen, filename);
			if (status < 1)
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
				printf("%10d %s\n",
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

	if (!diro->inode_read) {
		status = ext4fs_read_inode(diro->data, diro->ino, &diro->inode);
		if (status == 0)
			return 0;
	}
	symlink = zalloc(__le32_to_cpu(diro->inode.size) + 1);
	if (!symlink)
		return 0;

	if (__le32_to_cpu(diro->inode.size) <= 60) {
		strncpy(symlink, diro->inode.b.symlink,
			 __le32_to_cpu(diro->inode.size));
	} else {
		status = ext4fs_read_file(diro, 0,
					   __le32_to_cpu(diro->inode.size),
					   symlink);
		if (status == 0) {
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

int ext4fs_open(const char *filename)
{
	struct ext2fs_node *fdiro = NULL;
	int status;
	int len;

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
	len = __le32_to_cpu(fdiro->inode.size);
	ext4fs_file = fdiro;

	return len;
fail:
	ext4fs_free_node(fdiro, &ext4fs_root->diropen);

	return -1;
}

int ext4fs_mount(unsigned part_length)
{
	struct ext2_data *data;
	int status;
	struct ext_filesystem *fs = get_fs();
	data = zalloc(sizeof(struct ext2_data));
	if (!data)
		return 0;

	/* Read the superblock. */
	status = ext4fs_devread(1 * 2, 0, sizeof(struct ext2_sblock),
				(char *)&data->sblock);

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
