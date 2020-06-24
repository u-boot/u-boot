/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * From linux/fs/btrfs/ctree.h
 *   Copyright (C) 2007,2008 Oracle.  All rights reserved.
 *
 * Modified in 2017 by Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#ifndef __BTRFS_CTREE_H__
#define __BTRFS_CTREE_H__

#include <common.h>
#include <compiler.h>
#include "kernel-shared/btrfs_tree.h"
#include "compat.h"

#define BTRFS_MAX_MIRRORS 3

/*
 * the max metadata block size.  This limit is somewhat artificial,
 * but the memmove costs go through the roof for larger blocks.
 */
#define BTRFS_MAX_METADATA_BLOCKSIZE 65536

/*
 * Theoretical limit is larger, but we keep this down to a sane
 * value. That should limit greatly the possibility of collisions on
 * inode ref items.
 */
#define BTRFS_LINK_MAX 65535U

/* four bytes for CRC32 */
#define BTRFS_EMPTY_DIR_SIZE 0

/* ioprio of readahead is set to idle */
#define BTRFS_IOPRIO_READA (IOPRIO_PRIO_VALUE(IOPRIO_CLASS_IDLE, 0))

#define BTRFS_DIRTY_METADATA_THRESH	SZ_32M

#define BTRFS_MAX_EXTENT_SIZE SZ_128M

/*
 * File system states
 */
#define BTRFS_FS_STATE_ERROR		0
#define BTRFS_FS_STATE_REMOUNTING	1
#define BTRFS_FS_STATE_TRANS_ABORTED	2
#define BTRFS_FS_STATE_DEV_REPLACING	3
#define BTRFS_FS_STATE_DUMMY_FS_INFO	4

#define BTRFS_SETGET_STACK_FUNCS(name, type, member, bits)		\
static inline u##bits btrfs_##name(const type *s)			\
{									\
	return le##bits##_to_cpu(s->member);				\
}									\
static inline void btrfs_set_##name(type *s, u##bits val)		\
{									\
	s->member = cpu_to_le##bits(val);				\
}

union btrfs_tree_node {
	struct btrfs_header header;
	struct btrfs_leaf leaf;
	struct btrfs_node node;
};

struct btrfs_path {
	union btrfs_tree_node *nodes[BTRFS_MAX_LEVEL];
	u32 slots[BTRFS_MAX_LEVEL];
};

struct btrfs_root {
	u64 objectid;
	u64 bytenr;
	u64 root_dirid;
};

int btrfs_comp_keys(struct btrfs_key *, struct btrfs_key *);
int btrfs_comp_keys_type(struct btrfs_key *, struct btrfs_key *);
int btrfs_bin_search(union btrfs_tree_node *, struct btrfs_key *, int *);
void btrfs_free_path(struct btrfs_path *);
int btrfs_search_tree(const struct btrfs_root *, struct btrfs_key *,
		      struct btrfs_path *);
int btrfs_prev_slot(struct btrfs_path *);
int btrfs_next_slot(struct btrfs_path *);

static inline struct btrfs_key *btrfs_path_leaf_key(struct btrfs_path *p) {
	/* At tree read time we have converted the endian for btrfs_disk_key */
	return (struct btrfs_key *)&p->nodes[0]->leaf.items[p->slots[0]].key;
}

static inline struct btrfs_key *
btrfs_search_tree_key_type(const struct btrfs_root *root, u64 objectid,
			   u8 type, struct btrfs_path *path)
{
	struct btrfs_key key, *res;

	key.objectid = objectid;
	key.type = type;
	key.offset = 0;

	if (btrfs_search_tree(root, &key, path))
		return NULL;

	res = btrfs_path_leaf_key(path);
	if (btrfs_comp_keys_type(&key, res)) {
		btrfs_free_path(path);
		return NULL;
	}

	return res;
}

static inline u32 btrfs_path_item_size(struct btrfs_path *p)
{
	return p->nodes[0]->leaf.items[p->slots[0]].size;
}

static inline void *btrfs_leaf_data(struct btrfs_leaf *leaf, u32 slot)
{
	return ((u8 *) leaf) + sizeof(struct btrfs_header)
	       + leaf->items[slot].offset;
}

static inline void *btrfs_path_leaf_data(struct btrfs_path *p)
{
	return btrfs_leaf_data(&p->nodes[0]->leaf, p->slots[0]);
}

#define btrfs_item_ptr(l,s,t)			\
	((t *) btrfs_leaf_data((l),(s)))

#define btrfs_path_item_ptr(p,t)		\
	((t *) btrfs_path_leaf_data((p)))

u16 btrfs_super_csum_size(const struct btrfs_super_block *s);
const char *btrfs_super_csum_name(u16 csum_type);
u16 btrfs_csum_type_size(u16 csum_type);
size_t btrfs_super_num_csums(void);

/* struct btrfs_super_block */

BTRFS_SETGET_STACK_FUNCS(super_bytenr, struct btrfs_super_block, bytenr, 64);
BTRFS_SETGET_STACK_FUNCS(super_flags, struct btrfs_super_block, flags, 64);
BTRFS_SETGET_STACK_FUNCS(super_generation, struct btrfs_super_block,
			 generation, 64);
BTRFS_SETGET_STACK_FUNCS(super_root, struct btrfs_super_block, root, 64);
BTRFS_SETGET_STACK_FUNCS(super_sys_array_size,
			 struct btrfs_super_block, sys_chunk_array_size, 32);
BTRFS_SETGET_STACK_FUNCS(super_chunk_root_generation,
			 struct btrfs_super_block, chunk_root_generation, 64);
BTRFS_SETGET_STACK_FUNCS(super_root_level, struct btrfs_super_block,
			 root_level, 8);
BTRFS_SETGET_STACK_FUNCS(super_chunk_root, struct btrfs_super_block,
			 chunk_root, 64);
BTRFS_SETGET_STACK_FUNCS(super_chunk_root_level, struct btrfs_super_block,
			 chunk_root_level, 8);
BTRFS_SETGET_STACK_FUNCS(super_log_root, struct btrfs_super_block,
			 log_root, 64);
BTRFS_SETGET_STACK_FUNCS(super_log_root_transid, struct btrfs_super_block,
			 log_root_transid, 64);
BTRFS_SETGET_STACK_FUNCS(super_log_root_level, struct btrfs_super_block,
			 log_root_level, 8);
BTRFS_SETGET_STACK_FUNCS(super_total_bytes, struct btrfs_super_block,
			 total_bytes, 64);
BTRFS_SETGET_STACK_FUNCS(super_bytes_used, struct btrfs_super_block,
			 bytes_used, 64);
BTRFS_SETGET_STACK_FUNCS(super_sectorsize, struct btrfs_super_block,
			 sectorsize, 32);
BTRFS_SETGET_STACK_FUNCS(super_nodesize, struct btrfs_super_block,
			 nodesize, 32);
BTRFS_SETGET_STACK_FUNCS(super_stripesize, struct btrfs_super_block,
			 stripesize, 32);
BTRFS_SETGET_STACK_FUNCS(super_root_dir, struct btrfs_super_block,
			 root_dir_objectid, 64);
BTRFS_SETGET_STACK_FUNCS(super_num_devices, struct btrfs_super_block,
			 num_devices, 64);
BTRFS_SETGET_STACK_FUNCS(super_compat_flags, struct btrfs_super_block,
			 compat_flags, 64);
BTRFS_SETGET_STACK_FUNCS(super_compat_ro_flags, struct btrfs_super_block,
			 compat_ro_flags, 64);
BTRFS_SETGET_STACK_FUNCS(super_incompat_flags, struct btrfs_super_block,
			 incompat_flags, 64);
BTRFS_SETGET_STACK_FUNCS(super_csum_type, struct btrfs_super_block,
			 csum_type, 16);
BTRFS_SETGET_STACK_FUNCS(super_cache_generation, struct btrfs_super_block,
			 cache_generation, 64);
BTRFS_SETGET_STACK_FUNCS(super_uuid_tree_generation, struct btrfs_super_block,
			 uuid_tree_generation, 64);
BTRFS_SETGET_STACK_FUNCS(super_magic, struct btrfs_super_block, magic, 64);

#endif /* __BTRFS_CTREE_H__ */
