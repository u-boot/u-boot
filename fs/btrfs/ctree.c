// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <log.h>
#include <malloc.h>
#include <memalign.h>

static const struct btrfs_csum {
	u16 size;
	const char name[14];
} btrfs_csums[] = {
	[BTRFS_CSUM_TYPE_CRC32]		= {  4, "crc32c" },
	[BTRFS_CSUM_TYPE_XXHASH]	= {  8, "xxhash64" },
	[BTRFS_CSUM_TYPE_SHA256]	= { 32, "sha256" },
	[BTRFS_CSUM_TYPE_BLAKE2]	= { 32, "blake2" },
};

u16 btrfs_super_csum_size(const struct btrfs_super_block *sb)
{
	const u16 csum_type = btrfs_super_csum_type(sb);

	return btrfs_csums[csum_type].size;
}

const char *btrfs_super_csum_name(u16 csum_type)
{
	return btrfs_csums[csum_type].name;
}

size_t btrfs_super_num_csums(void)
{
	return ARRAY_SIZE(btrfs_csums);
}

u16 btrfs_csum_type_size(u16 csum_type)
{
	return btrfs_csums[csum_type].size;
}

int __btrfs_comp_keys(struct btrfs_key *a, struct btrfs_key *b)
{
	if (a->objectid > b->objectid)
		return 1;
	if (a->objectid < b->objectid)
		return -1;
	if (a->type > b->type)
		return 1;
	if (a->type < b->type)
		return -1;
	if (a->offset > b->offset)
		return 1;
	if (a->offset < b->offset)
		return -1;
	return 0;
}

int btrfs_comp_keys_type(struct btrfs_key *a, struct btrfs_key *b)
{
	if (a->objectid > b->objectid)
		return 1;
	if (a->objectid < b->objectid)
		return -1;
	if (a->type > b->type)
		return 1;
	if (a->type < b->type)
		return -1;
	return 0;
}

static int generic_bin_search(void *addr, int item_size, struct btrfs_key *key,
			      int max, int *slot)
{
	int low = 0, high = max, mid, ret;
	struct btrfs_key *tmp;

	while (low < high) {
		mid = (low + high) / 2;

		tmp = (struct btrfs_key *) ((u8 *) addr + mid*item_size);
		ret = __btrfs_comp_keys(tmp, key);

		if (ret < 0) {
			low = mid + 1;
		} else if (ret > 0) {
			high = mid;
		} else {
			*slot = mid;
			return 0;
		}
	}

	*slot = low;
	return 1;
}

int btrfs_bin_search(union btrfs_tree_node *p, struct btrfs_key *key,
		     int *slot)
{
	void *addr;
	unsigned long size;

	if (p->header.level) {
		addr = p->node.ptrs;
		size = sizeof(struct btrfs_key_ptr);
	} else {
		addr = p->leaf.items;
		size = sizeof(struct btrfs_item);
	}

	return generic_bin_search(addr, size, key, p->header.nritems, slot);
}

static void clear_path(struct __btrfs_path *p)
{
	int i;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		p->nodes[i] = NULL;
		p->slots[i] = 0;
	}
}

void __btrfs_free_path(struct __btrfs_path *p)
{
	int i;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		if (p->nodes[i])
			free(p->nodes[i]);
	}

	clear_path(p);
}

static int read_tree_node(u64 physical, union btrfs_tree_node **buf)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct btrfs_header, hdr,
				 sizeof(struct btrfs_header));
	unsigned long size, offset = sizeof(*hdr);
	union btrfs_tree_node *res;
	u32 i;

	if (!btrfs_devread(physical, sizeof(*hdr), hdr))
		return -1;

	btrfs_header_to_cpu(hdr);

	if (hdr->level)
		size = sizeof(struct btrfs_node)
		       + hdr->nritems * sizeof(struct btrfs_key_ptr);
	else
		size = btrfs_info.sb.nodesize;

	res = malloc_cache_aligned(size);
	if (!res) {
		debug("%s: malloc failed\n", __func__);
		return -1;
	}

	if (!btrfs_devread(physical + offset, size - offset,
			   ((u8 *) res) + offset)) {
		free(res);
		return -1;
	}

	memcpy(&res->header, hdr, sizeof(*hdr));
	if (hdr->level)
		for (i = 0; i < hdr->nritems; ++i)
			btrfs_key_ptr_to_cpu(&res->node.ptrs[i]);
	else
		for (i = 0; i < hdr->nritems; ++i)
			btrfs_item_to_cpu(&res->leaf.items[i]);

	*buf = res;

	return 0;
}

int btrfs_search_tree(const struct __btrfs_root *root, struct btrfs_key *key,
		      struct __btrfs_path *p)
{
	u8 lvl, prev_lvl;
	int i, slot, ret;
	u64 logical, physical;
	union btrfs_tree_node *buf;

	clear_path(p);

	logical = root->bytenr;

	for (i = 0; i < BTRFS_MAX_LEVEL; ++i) {
		physical = btrfs_map_logical_to_physical(logical);
		if (physical == -1ULL)
			goto err;

		if (read_tree_node(physical, &buf))
			goto err;

		lvl = buf->header.level;
		if (i && prev_lvl != lvl + 1) {
			printf("%s: invalid level in header at %llu\n",
			       __func__, logical);
			goto err;
		}
		prev_lvl = lvl;

		ret = btrfs_bin_search(buf, key, &slot);
		if (ret < 0)
			goto err;
		if (ret && slot > 0 && lvl)
			slot -= 1;

		p->slots[lvl] = slot;
		p->nodes[lvl] = buf;

		if (lvl) {
			logical = buf->node.ptrs[slot].blockptr;
		} else {
			/*
			 * The path might be invalid if:
			 *   cur leaf max < searched value < next leaf min
			 *
			 * Jump to the next valid element if it exists.
			 */
			if (slot >= buf->header.nritems)
				if (btrfs_next_slot(p) < 0)
					goto err;
			break;
		}
	}

	return 0;
err:
	__btrfs_free_path(p);
	return -1;
}

static int jump_leaf(struct __btrfs_path *path, int dir)
{
	struct __btrfs_path p;
	u32 slot;
	int level = 1, from_level, i;

	dir = dir >= 0 ? 1 : -1;

	p = *path;

	while (level < BTRFS_MAX_LEVEL) {
		if (!p.nodes[level])
			return 1;

		slot = p.slots[level];
		if ((dir > 0 && slot + dir >= p.nodes[level]->header.nritems)
		    || (dir < 0 && !slot))
			level++;
		else
			break;
	}

	if (level == BTRFS_MAX_LEVEL)
		return 1;

	p.slots[level] = slot + dir;
	level--;
	from_level = level;

	while (level >= 0) {
		u64 logical, physical;

		slot = p.slots[level + 1];
		logical = p.nodes[level + 1]->node.ptrs[slot].blockptr;
		physical = btrfs_map_logical_to_physical(logical);
		if (physical == -1ULL)
			goto err;

		if (read_tree_node(physical, &p.nodes[level]))
			goto err;

		if (dir > 0)
			p.slots[level] = 0;
		else
			p.slots[level] = p.nodes[level]->header.nritems - 1;
		level--;
	}

	/* Free rewritten nodes in path */
	for (i = 0; i <= from_level; ++i)
		free(path->nodes[i]);

	*path = p;
	return 0;

err:
	/* Free rewritten nodes in p */
	for (i = level + 1; i <= from_level; ++i)
		free(p.nodes[i]);
	return -1;
}

int btrfs_prev_slot(struct __btrfs_path *p)
{
	if (!p->slots[0])
		return jump_leaf(p, -1);

	p->slots[0]--;
	return 0;
}

int btrfs_next_slot(struct __btrfs_path *p)
{
	struct btrfs_leaf *leaf = &p->nodes[0]->leaf;

	if (p->slots[0] + 1 >= leaf->header.nritems)
		return jump_leaf(p, 1);

	p->slots[0]++;
	return 0;
}

int btrfs_comp_cpu_keys(const struct btrfs_key *k1, const struct btrfs_key *k2)
{
	if (k1->objectid > k2->objectid)
		return 1;
	if (k1->objectid < k2->objectid)
		return -1;
	if (k1->type > k2->type)
		return 1;
	if (k1->type < k2->type)
		return -1;
	if (k1->offset > k2->offset)
		return 1;
	if (k1->offset < k2->offset)
		return -1;
	return 0;
}

static int btrfs_comp_keys(struct btrfs_disk_key *disk,
			     const struct btrfs_key *k2)
{
	struct btrfs_key k1;

	btrfs_disk_key_to_cpu(&k1, disk);
	return btrfs_comp_cpu_keys(&k1, k2);
}

enum btrfs_tree_block_status
btrfs_check_node(struct btrfs_fs_info *fs_info,
		 struct btrfs_disk_key *parent_key, struct extent_buffer *buf)
{
	int i;
	struct btrfs_key cpukey;
	struct btrfs_disk_key key;
	u32 nritems = btrfs_header_nritems(buf);
	enum btrfs_tree_block_status ret = BTRFS_TREE_BLOCK_INVALID_NRITEMS;

	if (nritems == 0 || nritems > BTRFS_NODEPTRS_PER_BLOCK(fs_info))
		goto fail;

	ret = BTRFS_TREE_BLOCK_INVALID_PARENT_KEY;
	if (parent_key && parent_key->type) {
		btrfs_node_key(buf, &key, 0);
		if (memcmp(parent_key, &key, sizeof(key)))
			goto fail;
	}
	ret = BTRFS_TREE_BLOCK_BAD_KEY_ORDER;
	for (i = 0; nritems > 1 && i < nritems - 2; i++) {
		btrfs_node_key(buf, &key, i);
		btrfs_node_key_to_cpu(buf, &cpukey, i + 1);
		if (btrfs_comp_keys(&key, &cpukey) >= 0)
			goto fail;
	}
	return BTRFS_TREE_BLOCK_CLEAN;
fail:
	return ret;
}

enum btrfs_tree_block_status
btrfs_check_leaf(struct btrfs_fs_info *fs_info,
		 struct btrfs_disk_key *parent_key, struct extent_buffer *buf)
{
	int i;
	struct btrfs_key cpukey;
	struct btrfs_disk_key key;
	u32 nritems = btrfs_header_nritems(buf);
	enum btrfs_tree_block_status ret = BTRFS_TREE_BLOCK_INVALID_NRITEMS;

	if (nritems * sizeof(struct btrfs_item) > buf->len)  {
		fprintf(stderr, "invalid number of items %llu\n",
			(unsigned long long)buf->start);
		goto fail;
	}

	if (btrfs_header_level(buf) != 0) {
		ret = BTRFS_TREE_BLOCK_INVALID_LEVEL;
		fprintf(stderr, "leaf is not a leaf %llu\n",
		       (unsigned long long)btrfs_header_bytenr(buf));
		goto fail;
	}
	if (btrfs_leaf_free_space(buf) < 0) {
		ret = BTRFS_TREE_BLOCK_INVALID_FREE_SPACE;
		fprintf(stderr, "leaf free space incorrect %llu %d\n",
			(unsigned long long)btrfs_header_bytenr(buf),
			btrfs_leaf_free_space(buf));
		goto fail;
	}

	if (nritems == 0)
		return BTRFS_TREE_BLOCK_CLEAN;

	btrfs_item_key(buf, &key, 0);
	if (parent_key && parent_key->type &&
	    memcmp(parent_key, &key, sizeof(key))) {
		ret = BTRFS_TREE_BLOCK_INVALID_PARENT_KEY;
		fprintf(stderr, "leaf parent key incorrect %llu\n",
		       (unsigned long long)btrfs_header_bytenr(buf));
		goto fail;
	}
	for (i = 0; nritems > 1 && i < nritems - 1; i++) {
		btrfs_item_key(buf, &key, i);
		btrfs_item_key_to_cpu(buf, &cpukey, i + 1);
		if (btrfs_comp_keys(&key, &cpukey) >= 0) {
			ret = BTRFS_TREE_BLOCK_BAD_KEY_ORDER;
			fprintf(stderr, "bad key ordering %d %d\n", i, i+1);
			goto fail;
		}
		if (btrfs_item_offset_nr(buf, i) !=
			btrfs_item_end_nr(buf, i + 1)) {
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "incorrect offsets %u %u\n",
				btrfs_item_offset_nr(buf, i),
				btrfs_item_end_nr(buf, i + 1));
			goto fail;
		}
		if (i == 0 && btrfs_item_end_nr(buf, i) !=
		    BTRFS_LEAF_DATA_SIZE(fs_info)) {
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "bad item end %u wanted %u\n",
				btrfs_item_end_nr(buf, i),
				(unsigned)BTRFS_LEAF_DATA_SIZE(fs_info));
			goto fail;
		}
	}

	for (i = 0; i < nritems; i++) {
		if (btrfs_item_end_nr(buf, i) >
				BTRFS_LEAF_DATA_SIZE(fs_info)) {
			btrfs_item_key(buf, &key, 0);
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "slot end outside of leaf %llu > %llu\n",
				(unsigned long long)btrfs_item_end_nr(buf, i),
				(unsigned long long)BTRFS_LEAF_DATA_SIZE(
					fs_info));
			goto fail;
		}
	}

	return BTRFS_TREE_BLOCK_CLEAN;
fail:
	return ret;
}

/*
 * how many bytes are required to store the items in a leaf.  start
 * and nr indicate which items in the leaf to check.  This totals up the
 * space used both by the item structs and the item data
 */
static int leaf_space_used(struct extent_buffer *l, int start, int nr)
{
	int data_len;
	int nritems = btrfs_header_nritems(l);
	int end = min(nritems, start + nr) - 1;

	if (!nr)
		return 0;
	data_len = btrfs_item_end_nr(l, start);
	data_len = data_len - btrfs_item_offset_nr(l, end);
	data_len += sizeof(struct btrfs_item) * nr;
	WARN_ON(data_len < 0);
	return data_len;
}

/*
 * The space between the end of the leaf items and
 * the start of the leaf data.  IOW, how much room
 * the leaf has left for both items and data
 */
int btrfs_leaf_free_space(struct extent_buffer *leaf)
{
	int nritems = btrfs_header_nritems(leaf);
	u32 leaf_data_size;
	int ret;

	BUG_ON(leaf->fs_info && leaf->fs_info->nodesize != leaf->len);
	leaf_data_size = __BTRFS_LEAF_DATA_SIZE(leaf->len);
	ret = leaf_data_size - leaf_space_used(leaf, 0 ,nritems);
	if (ret < 0) {
		printk("leaf free space ret %d, leaf data size %u, used %d nritems %d\n",
		       ret, leaf_data_size, leaf_space_used(leaf, 0, nritems),
		       nritems);
	}
	return ret;
}
