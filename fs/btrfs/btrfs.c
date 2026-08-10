// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include <config.h>
#include <malloc.h>
#include <u-boot/uuid.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <fs.h>
#include "btrfs.h"
#include "crypto/hash.h"
#include "disk-io.h"

struct btrfs_fs_info *current_fs_info;

int btrfs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition)
{
	struct btrfs_fs_info *fs_info;
	int ret = -1;

	btrfs_hash_init();
	fs_info = open_ctree_fs_info(fs_dev_desc, fs_partition);
	if (fs_info) {
		current_fs_info = fs_info;
		ret = 0;
	}
	return ret;
}

/*
 * The fs layer closes and re-probes btrfs between readdir() calls (see
 * fs_readdir() in fs/fs.c), freeing and reallocating fs_info, so root cannot
 * be stored directly. The subvolume id and inode number are stable though, so
 * re-resolve the root from the current fs_info by subvolume id, which avoids
 * a full path walk and is much faster.
 */
struct btrfs_dir_stream {
	struct fs_dir_stream parent;
	struct fs_dirent dirent;
	u64 subvolid;
	u64 ino;
	u64 offset;
};

int btrfs_opendir(const char *dirname, struct fs_dir_stream **dirsp)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct btrfs_dir_stream *dirs;
	struct btrfs_root *root;
	u64 ino;
	u8 type;
	int ret;

	*dirsp = NULL;
	ASSERT(fs_info);

	ret = btrfs_lookup_path(fs_info->fs_root, BTRFS_FIRST_FREE_OBJECTID,
				dirname, &root, &ino, &type, 40);
	if (ret < 0)
		return ret;
	if (type != BTRFS_FT_DIR)
		return -ENOTDIR;

	dirs = calloc(1, sizeof(*dirs));
	if (!dirs)
		return -ENOMEM;
	dirs->subvolid = root->root_key.objectid;
	dirs->ino = ino;

	*dirsp = &dirs->parent;
	return 0;
}

static unsigned int btrfs_dirent_type_to_fs_type(u8 dirent_type)
{
	switch (dirent_type) {
	case BTRFS_FT_DIR:
		return FS_DT_DIR;
	case BTRFS_FT_SYMLINK:
		return FS_DT_LNK;
	default:
		return FS_DT_REG;
	}
}

int btrfs_readdir(struct fs_dir_stream *fs_dirs, struct fs_dirent **dentp)
{
	struct btrfs_dir_stream *dirs = container_of(fs_dirs, struct btrfs_dir_stream, parent);
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct fs_dirent *dent = &dirs->dirent;
	struct btrfs_root *root;
	struct btrfs_key key;
	u8 type;
	int ret;

	*dentp = NULL;
	ASSERT(fs_info);

	key.objectid = dirs->subvolid;
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = (u64)-1;
	root = btrfs_read_fs_root(fs_info, &key);
	if (IS_ERR(root))
		return PTR_ERR(root);

	memset(dent, 0, sizeof(*dent));
	ret = btrfs_next_dir_entry(root, dirs->ino, &dirs->offset, dent->name,
				   sizeof(dent->name), &type);
	if (ret < 0)
		return ret;
	if (ret > 0)
		return -ENOENT;

	dent->type = btrfs_dirent_type_to_fs_type(type);
	*dentp = dent;
	return 0;
}

void btrfs_closedir(struct fs_dir_stream *fs_dirs)
{
	struct btrfs_dir_stream *dirs = container_of(fs_dirs, struct btrfs_dir_stream, parent);

	free(dirs);
}

int btrfs_exists(const char *file)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct btrfs_root *root;
	u64 ino;
	u8 type;
	int ret;

	ASSERT(fs_info);

	ret = btrfs_lookup_path(fs_info->fs_root, BTRFS_FIRST_FREE_OBJECTID,
				file, &root, &ino, &type, 40);
	if (ret < 0)
		return 0;

	if (type == BTRFS_FT_REG_FILE)
		return 1;
	return 0;
}

int btrfs_size(const char *file, loff_t *size)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct btrfs_inode_item *ii;
	struct btrfs_root *root;
	struct btrfs_path path;
	struct btrfs_key key;
	u64 ino;
	u8 type;
	int ret;

	ret = btrfs_lookup_path(fs_info->fs_root, BTRFS_FIRST_FREE_OBJECTID,
				file, &root, &ino, &type, 40);
	if (ret < 0) {
		debug("Cannot lookup file %s\n", file);
		return ret;
	}
	if (type != BTRFS_FT_REG_FILE) {
		printf("Not a regular file: %s\n", file);
		return -ENOENT;
	}
	btrfs_init_path(&path);
	key.objectid = ino;
	key.type = BTRFS_INODE_ITEM_KEY;
	key.offset = 0;

	ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
	if (ret < 0) {
		printf("Cannot lookup ino %llu\n", ino);
		return ret;
	}
	if (ret > 0) {
		printf("Ino %llu does not exist\n", ino);
		ret = -ENOENT;
		goto out;
	}
	ii = btrfs_item_ptr(path.nodes[0], path.slots[0],
			    struct btrfs_inode_item);
	*size = btrfs_inode_size(path.nodes[0], ii);
out:
	btrfs_release_path(&path);
	return ret;
}

int btrfs_read(const char *file, void *buf, loff_t offset, loff_t len,
	       loff_t *actread)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct btrfs_root *root;
	loff_t real_size;
	u64 ino;
	u8 type;
	int ret;

	ASSERT(fs_info);
	ret = btrfs_lookup_path(fs_info->fs_root, BTRFS_FIRST_FREE_OBJECTID,
				file, &root, &ino, &type, 40);
	if (ret < 0) {
		error("Cannot lookup file %s", file);
		return ret;
	}

	if (type != BTRFS_FT_REG_FILE) {
		error("Not a regular file: %s", file);
		return -EINVAL;
	}

	ret = btrfs_size(file, &real_size);
	if (ret < 0) {
		error("Failed to get inode size: %s", file);
		return ret;
	}

	if (!len || len > real_size - offset)
		len = real_size - offset;

	ret = btrfs_file_read(root, ino, offset, len, buf);
	if (ret < 0) {
		error("An error occurred while reading file %s", file);
		return ret;
	}

	*actread = len;
	return 0;
}

void btrfs_close(void)
{
	if (current_fs_info) {
		close_ctree_fs_info(current_fs_info);
		current_fs_info = NULL;
	}
}

int btrfs_uuid(char *uuid_str)
{
#ifdef CONFIG_LIB_UUID
	if (current_fs_info)
		uuid_bin_to_str(current_fs_info->super_copy->fsid, uuid_str,
				UUID_STR_FORMAT_STD);
	return 0;
#endif
	return -ENOSYS;
}
