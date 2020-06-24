// SPDX-License-Identifier: GPL-2.0+
#ifndef __BTRFS_DISK_IO_H__
#define __BTRFS_DISK_IO_H__

#include <linux/sizes.h>
#include <fs_internal.h>
#include "ctree.h"
#include "disk-io.h"

#define BTRFS_SUPER_INFO_OFFSET SZ_64K
#define BTRFS_SUPER_INFO_SIZE	SZ_4K

/* U-boot specific */
int btrfs_read_superblock(void);

/* From btrfs-progs */
int read_whole_eb(struct btrfs_fs_info *info, struct extent_buffer *eb, int mirror);
struct extent_buffer* read_tree_block(struct btrfs_fs_info *fs_info, u64 bytenr,
		u64 parent_transid);

struct extent_buffer* btrfs_find_create_tree_block(
		struct btrfs_fs_info *fs_info, u64 bytenr);
struct extent_buffer *btrfs_find_tree_block(struct btrfs_fs_info *fs_info,
					    u64 bytenr, u32 blocksize);
int btrfs_read_dev_super(struct blk_desc *desc, struct disk_partition *part,
			 struct btrfs_super_block *sb);
int btrfs_buffer_uptodate(struct extent_buffer *buf, u64 parent_transid);
int btrfs_set_buffer_uptodate(struct extent_buffer *buf);
int btrfs_csum_data(u16 csum_type, const u8 *data, u8 *out, size_t len);
int csum_tree_block_size(struct extent_buffer *buf, u16 csum_sectorsize,
			 int verify, u16 csum_type);
#endif
