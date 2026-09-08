// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#include <asm/unaligned.h>
#include <compiler.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqfs_decompressor.h"
#include "sqfs_filesystem.h"
#include "sqfs_utils.h"

int sqfs_inode_size(struct squashfs_base_inode *inode, u32 blk_size, size_t max)
{
	u16 inode_type;

	/* The smallest possible inode must fit in the remaining bytes */
	if (max < sizeof(struct squashfs_base_inode))
		return -EINVAL;

	inode_type = get_unaligned_le16(&inode->inode_type);

	switch (inode_type) {
	case SQFS_DIR_TYPE:
		if (max < sizeof(struct squashfs_dir_inode))
			return -EINVAL;
		return sizeof(struct squashfs_dir_inode);

	case SQFS_REG_TYPE: {
		struct squashfs_reg_inode *reg =
			(struct squashfs_reg_inode *)inode;
		u32 fragment, file_size;
		unsigned int blk_list_size;
		int size;

		if (max < sizeof(*reg))
			return -EINVAL;

		fragment = get_unaligned_le32(&reg->fragment);
		file_size = get_unaligned_le32(&reg->file_size);

		if (!blk_size)
			return -EINVAL;

		if (SQFS_IS_FRAGMENTED(fragment))
			blk_list_size = file_size / blk_size;
		else
			blk_list_size = DIV_ROUND_UP(file_size, blk_size);

		if (__builtin_mul_overflow(blk_list_size, (unsigned int)sizeof(u32),
					   &blk_list_size) ||
		    __builtin_add_overflow((int)sizeof(*reg), (int)blk_list_size,
					   &size))
			return -EINVAL;

		return size;
	}

	case SQFS_LDIR_TYPE: {
		struct squashfs_ldir_inode *ldir =
			(struct squashfs_ldir_inode *)inode;
		u16 i_count;
		unsigned int index_list_size = 0, l = 0;
		struct squashfs_directory_index *di;
		size_t consumed;
		u32 sz;
		int size;

		if (max < sizeof(*ldir))
			return -EINVAL;

		i_count = get_unaligned_le16(&ldir->i_count);
		if (i_count == 0)
			return sizeof(*ldir);

		di = ldir->index;
		consumed = sizeof(*ldir);
		while (l < i_count) {
			/* The directory index header must stay in bounds */
			if (consumed + sizeof(*di) > max)
				return -EINVAL;
			sz = get_unaligned_le32(&di->size) + 1;
			if (__builtin_add_overflow(consumed, sizeof(*di) + sz,
						   &consumed) ||
			    consumed > max)
				return -EINVAL;
			index_list_size += sz;
			di = (void *)di + sizeof(*di) + sz;
			l++;
		}

		if (__builtin_add_overflow((int)(sizeof(*ldir) + index_list_size),
					   (int)(i_count * SQFS_DIR_INDEX_BASE_LENGTH),
					   &size))
			return -EINVAL;

		return size;
	}

	case SQFS_LREG_TYPE: {
		struct squashfs_lreg_inode *lreg =
			(struct squashfs_lreg_inode *)inode;
		u32 fragment;
		u64 file_size;
		unsigned int blk_list_size;
		int size;

		if (max < sizeof(*lreg))
			return -EINVAL;

		fragment = get_unaligned_le32(&lreg->fragment);
		file_size = get_unaligned_le64(&lreg->file_size);

		if (!blk_size)
			return -EINVAL;

		if (fragment == 0xFFFFFFFF)
			blk_list_size = DIV_ROUND_UP(file_size, blk_size);
		else
			blk_list_size = file_size / blk_size;

		if (__builtin_mul_overflow(blk_list_size, (unsigned int)sizeof(u32),
					   &blk_list_size) ||
		    __builtin_add_overflow((int)sizeof(*lreg), (int)blk_list_size,
					   &size))
			return -EINVAL;

		return size;
	}

	case SQFS_SYMLINK_TYPE:
	case SQFS_LSYMLINK_TYPE: {
		int size;

		struct squashfs_symlink_inode *symlink =
			(struct squashfs_symlink_inode *)inode;

		if (max < sizeof(*symlink))
			return -EINVAL;

		if (__builtin_add_overflow(sizeof(*symlink),
		    get_unaligned_le32(&symlink->symlink_size), &size))
			return -EINVAL;

		return (inode_type == SQFS_SYMLINK_TYPE) ? size : size + sizeof(u32);
	}

	case SQFS_BLKDEV_TYPE:
	case SQFS_CHRDEV_TYPE:
		if (max < sizeof(struct squashfs_dev_inode))
			return -EINVAL;
		return sizeof(struct squashfs_dev_inode);
	case SQFS_LBLKDEV_TYPE:
	case SQFS_LCHRDEV_TYPE:
		if (max < sizeof(struct squashfs_ldev_inode))
			return -EINVAL;
		return sizeof(struct squashfs_ldev_inode);
	case SQFS_FIFO_TYPE:
	case SQFS_SOCKET_TYPE:
		if (max < sizeof(struct squashfs_ipc_inode))
			return -EINVAL;
		return sizeof(struct squashfs_ipc_inode);
	case SQFS_LFIFO_TYPE:
	case SQFS_LSOCKET_TYPE:
		if (max < sizeof(struct squashfs_lipc_inode))
			return -EINVAL;
		return sizeof(struct squashfs_lipc_inode);
	default:
		printf("Error while searching inode: unknown type.\n");
		return -EINVAL;
	}
}

/*
 * Given the uncompressed inode table, the inode to be found and the number of
 * inodes in the table, return inode position in case of success.
 */
void *sqfs_find_inode(void *inode_table, size_t table_size, int inode_number,
		      __le32 inode_count, __le32 block_size)
{
	struct squashfs_base_inode *base;
	size_t offset = 0;
	unsigned int k;
	int sz;

	if (!inode_table) {
		printf("%s: Invalid pointer to inode table.\n", __func__);
		return NULL;
	}

	for (k = 0; k < le32_to_cpu(inode_count); k++) {
		/* The base inode header must lie within the inode table */
		if (offset + sizeof(struct squashfs_base_inode) > table_size)
			return NULL;

		base = inode_table + offset;
		if (get_unaligned_le32(&base->inode_number) == inode_number)
			return inode_table + offset;

		sz = sqfs_inode_size(base, le32_to_cpu(block_size),
				     table_size - offset);
		if (sz <= 0 || (size_t)sz > table_size - offset)
			return NULL;

		offset += sz;
	}

	printf("Inode not found.\n");

	return NULL;
}

int sqfs_read_metablock(unsigned char *file_mapping, int offset,
			bool *compressed, u32 *data_size)
{
	const unsigned char *data;
	u16 header;

	if (!file_mapping)
		return -EFAULT;
	data = file_mapping + offset;

	header = get_unaligned((u16 *)data);
	if (!header)
		return -EINVAL;

	*compressed = SQFS_COMPRESSED_METADATA(header);
	*data_size = SQFS_METADATA_SIZE(header);

	if (*data_size > SQFS_METADATA_BLOCK_SIZE) {
		printf("Invalid metatada block size: %d bytes.\n", *data_size);
		return -EINVAL;
	}

	return 0;
}
