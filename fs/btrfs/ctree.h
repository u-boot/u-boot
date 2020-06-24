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

#endif /* __BTRFS_CTREE_H__ */
