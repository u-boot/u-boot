// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include "disk-io.h"

static int verify_dir_item(struct btrfs_root *root,
		    struct extent_buffer *leaf,
		    struct btrfs_dir_item *dir_item)
{
	u16 namelen = BTRFS_NAME_LEN;
	u8 type = btrfs_dir_type(leaf, dir_item);

	if (type == BTRFS_FT_XATTR)
		namelen = XATTR_NAME_MAX;

	if (btrfs_dir_name_len(leaf, dir_item) > namelen) {
		fprintf(stderr, "invalid dir item name len: %u\n",
			(unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	/* BTRFS_MAX_XATTR_SIZE is the same for all dir items */
	if ((btrfs_dir_data_len(leaf, dir_item) +
	     btrfs_dir_name_len(leaf, dir_item)) >
			BTRFS_MAX_XATTR_SIZE(root->fs_info)) {
		fprintf(stderr, "invalid dir item name + data len: %u + %u\n",
			(unsigned)btrfs_dir_name_len(leaf, dir_item),
			(unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	return 0;
}

struct btrfs_dir_item *btrfs_match_dir_item_name(struct btrfs_root *root,
			      struct btrfs_path *path,
			      const char *name, int name_len)
{
	struct btrfs_dir_item *dir_item;
	unsigned long name_ptr;
	u32 total_len;
	u32 cur = 0;
	u32 this_len;
	struct extent_buffer *leaf;

	leaf = path->nodes[0];
	dir_item = btrfs_item_ptr(leaf, path->slots[0], struct btrfs_dir_item);
	total_len = btrfs_item_size_nr(leaf, path->slots[0]);
	if (verify_dir_item(root, leaf, dir_item))
		return NULL;

	while(cur < total_len) {
		this_len = sizeof(*dir_item) +
			btrfs_dir_name_len(leaf, dir_item) +
			btrfs_dir_data_len(leaf, dir_item);
		if (this_len > (total_len - cur)) {
			fprintf(stderr, "invalid dir item size\n");
			return NULL;
		}

		name_ptr = (unsigned long)(dir_item + 1);

		if (btrfs_dir_name_len(leaf, dir_item) == name_len &&
		    memcmp_extent_buffer(leaf, name, name_ptr, name_len) == 0)
			return dir_item;

		cur += this_len;
		dir_item = (struct btrfs_dir_item *)((char *)dir_item +
						     this_len);
	}
	return NULL;
}

struct btrfs_dir_item *btrfs_lookup_dir_item(struct btrfs_trans_handle *trans,
					     struct btrfs_root *root,
					     struct btrfs_path *path, u64 dir,
					     const char *name, int name_len,
					     int mod)
{
	int ret;
	struct btrfs_key key;
	int ins_len = mod < 0 ? -1 : 0;
	int cow = mod != 0;
	struct btrfs_key found_key;
	struct extent_buffer *leaf;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;

	key.offset = btrfs_name_hash(name, name_len);

	ret = btrfs_search_slot(trans, root, &key, path, ins_len, cow);
	if (ret < 0)
		return ERR_PTR(ret);
	if (ret > 0) {
		if (path->slots[0] == 0)
			return NULL;
		path->slots[0]--;
	}

	leaf = path->nodes[0];
	btrfs_item_key_to_cpu(leaf, &found_key, path->slots[0]);

	if (found_key.objectid != dir ||
	    found_key.type != BTRFS_DIR_ITEM_KEY ||
	    found_key.offset != key.offset)
		return NULL;

	return btrfs_match_dir_item_name(root, path, name, name_len);
}

static int __verify_dir_item(struct btrfs_dir_item *item, u32 start, u32 total)
{
	u16 max_len = BTRFS_NAME_LEN;
	u32 end;

	if (item->type >= BTRFS_FT_MAX) {
		printf("%s: invalid dir item type: %i\n", __func__, item->type);
		return 1;
	}

	if (item->type == BTRFS_FT_XATTR)
		max_len = 255; /* XATTR_NAME_MAX */

	end = start + sizeof(*item) + item->name_len;
	if (item->name_len > max_len || end > total) {
		printf("%s: invalid dir item name len: %u\n", __func__,
		       item->name_len);
		return 1;
	}

	return 0;
}

static struct btrfs_dir_item *
__btrfs_match_dir_item_name(struct __btrfs_path *path, const char *name,
			  int name_len)
{
	struct btrfs_dir_item *item;
	u32 total_len, cur = 0, this_len;
	const char *name_ptr;

	item = btrfs_path_item_ptr(path, struct btrfs_dir_item);

	total_len = btrfs_path_item_size(path);

	while (cur < total_len) {
		btrfs_dir_item_to_cpu(item);
		this_len = sizeof(*item) + item->name_len + item->data_len;
		name_ptr = (const char *) (item + 1);

		if (__verify_dir_item(item, cur, total_len))
			return NULL;
		if (item->name_len == name_len && !memcmp(name_ptr, name,
							  name_len))
			return item;

		cur += this_len;
		item = (struct btrfs_dir_item *) ((u8 *) item + this_len);
	}

	return NULL;
}

int __btrfs_lookup_dir_item(const struct __btrfs_root *root, u64 dir,
			  const char *name, int name_len,
			  struct btrfs_dir_item *item)
{
	struct __btrfs_path path;
	struct btrfs_key key;
	struct btrfs_dir_item *res = NULL;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);

	if (btrfs_search_tree(root, &key, &path))
		return -1;

	if (btrfs_comp_keys_type(&key, btrfs_path_leaf_key(&path)))
		goto out;

	res = __btrfs_match_dir_item_name(&path, name, name_len);
	if (res)
		*item = *res;
out:
	__btrfs_free_path(&path);
	return res ? 0 : -1;
}

int btrfs_readdir(const struct __btrfs_root *root, u64 dir,
		  btrfs_readdir_callback_t callback)
{
	struct __btrfs_path path;
	struct btrfs_key key, *found_key;
	struct btrfs_dir_item *item;
	int res = 0;

	key.objectid = dir;
	key.type = BTRFS_DIR_INDEX_KEY;
	key.offset = 0;

	if (btrfs_search_tree(root, &key, &path))
		return -1;

	do {
		found_key = btrfs_path_leaf_key(&path);
		if (btrfs_comp_keys_type(&key, found_key))
			break;

		item = btrfs_path_item_ptr(&path, struct btrfs_dir_item);
		btrfs_dir_item_to_cpu(item);

		if (__verify_dir_item(item, 0, sizeof(*item) + item->name_len))
			continue;
		if (item->type == BTRFS_FT_XATTR)
			continue;

		if (callback(root, item))
			break;
	} while (!(res = btrfs_next_slot(&path)));

	__btrfs_free_path(&path);

	return res < 0 ? -1 : 0;
}
