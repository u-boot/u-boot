/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#ifndef SQFS_UTILS_H
#define SQFS_UTILS_H

#include <linux/bitops.h>
#include <linux/kernel.h>
#include <stdbool.h>

#define SQFS_FRAGMENT_INDEX_OFFSET(A) ((A) % SQFS_MAX_ENTRIES)
#define SQFS_FRAGMENT_INDEX(A) ((A) / SQFS_MAX_ENTRIES)
#define SQFS_BLOCK_SIZE(A) ((A) & GENMASK(23, 0))
#define SQFS_CHECK_FLAG(flag, bit) (((flag) >> (bit)) & 1)
/* Useful for both fragment and data blocks */
#define SQFS_COMPRESSED_BLOCK(A) (!((A) & BIT(24)))
/* SQFS_COMPRESSED_DATA strictly used with super block's 'flags' member */
#define SQFS_COMPRESSED_DATA(A) (!((A) & 0x0002))
#define SQFS_IS_FRAGMENTED(A) ((A) != 0xFFFFFFFF)
/*
 * These two macros work as getters for a metada block header, retrieving the
 * data size and if it is compressed/uncompressed
 */
#define SQFS_COMPRESSED_METADATA(A) (!((A) & BIT(15)))
#define SQFS_METADATA_SIZE(A) ((A) & GENMASK(14, 0))

struct squashfs_super_block_flags {
	/* check: unused
	 * uncompressed_ids: not supported
	 */
	bool uncompressed_inodes;
	bool uncompressed_data;
	bool check;
	bool uncompressed_frags;
	bool no_frags;
	bool always_frags;
	bool duplicates;
	bool exportable;
	bool uncompressed_xattrs;
	bool no_xattrs;
	bool compressor_options;
	bool uncompressed_ids;
};

#endif /* SQFS_UTILS_H  */
