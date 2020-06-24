// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include <config.h>
#include <malloc.h>
#include <uuid.h>
#include <linux/time.h>
#include "btrfs.h"
#include "crypto/hash.h"
#include "disk-io.h"

struct btrfs_info btrfs_info;
struct btrfs_fs_info *current_fs_info;

static int show_dir(struct btrfs_root *root, struct extent_buffer *eb,
		    struct btrfs_dir_item *di)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_inode_item ii;
	struct btrfs_key key;
	static const char* dir_item_str[] = {
		[BTRFS_FT_REG_FILE]	= "FILE",
		[BTRFS_FT_DIR] 		= "DIR",
		[BTRFS_FT_CHRDEV]	= "CHRDEV",
		[BTRFS_FT_BLKDEV]	= "BLKDEV",
		[BTRFS_FT_FIFO]		= "FIFO",
		[BTRFS_FT_SOCK]		= "SOCK",
		[BTRFS_FT_SYMLINK]	= "SYMLINK",
		[BTRFS_FT_XATTR]	= "XATTR"
	};
	u8 type = btrfs_dir_type(eb, di);
	char namebuf[BTRFS_NAME_LEN];
	char *target = NULL;
	char filetime[32];
	time_t mtime;
	int ret;

	btrfs_dir_item_key_to_cpu(eb, di, &key);

	if (key.type == BTRFS_ROOT_ITEM_KEY) {
		struct btrfs_root *subvol;

		/* It's a subvolume, get its mtime from root item */
		subvol = btrfs_read_fs_root(fs_info, &key);
		if (IS_ERR(subvol)) {
			ret = PTR_ERR(subvol);
			error("Can't find root %llu", key.objectid);
			return ret;
		}
		mtime = btrfs_stack_timespec_sec(&subvol->root_item.otime);
	} else {
		struct btrfs_path path;

		/* It's regular inode, get its mtime from inode item */
		btrfs_init_path(&path);
		ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
		if (ret > 0)
			ret = -ENOENT;
		if (ret < 0) {
			error("Can't find inode %llu", key.objectid);
			btrfs_release_path(&path);
			return ret;
		}
		read_extent_buffer(path.nodes[0], &ii,
			btrfs_item_ptr_offset(path.nodes[0], path.slots[0]),
			sizeof(ii));
		btrfs_release_path(&path);
		mtime = btrfs_stack_timespec_sec(&ii.mtime);
	}
	ctime_r(&mtime, filetime);

	if (type == BTRFS_FT_SYMLINK) {
		target = malloc(fs_info->sectorsize);
		if (!target) {
			error("Can't alloc memory for symlink %llu",
				key.objectid);
			return -ENOMEM;
		}
		ret = btrfs_readlink(root, key.objectid, target);
		if (ret < 0) {
			error("Failed to read symlink %llu", key.objectid);
			goto out;
		}
		target[ret] = '\0';
	}

	if (type < ARRAY_SIZE(dir_item_str) && dir_item_str[type])
		printf("<%s> ", dir_item_str[type]);
	else
		printf("DIR_ITEM.%u", type);
	if (type == BTRFS_FT_CHRDEV || type == BTRFS_FT_BLKDEV) {
		ASSERT(key.type == BTRFS_INODE_ITEM_KEY);
		printf("%4llu,%5llu  ", btrfs_stack_inode_rdev(&ii) >> 20,
				btrfs_stack_inode_rdev(&ii) & 0xfffff);
	} else {
		if (key.type == BTRFS_INODE_ITEM_KEY)
			printf("%10llu  ", btrfs_stack_inode_size(&ii));
		else
			printf("%10llu  ", 0ULL);
	}

	read_extent_buffer(eb, namebuf, (unsigned long)(di + 1),
			   btrfs_dir_name_len(eb, di));
	printf("%24.24s  %.*s", filetime, btrfs_dir_name_len(eb, di), namebuf);
	if (type == BTRFS_FT_SYMLINK)
		printf(" -> %s", target ? target : "?");
	printf("\n");
out:
	free(target);
	return ret;
}

int btrfs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition)
{
	struct btrfs_fs_info *fs_info;
	int ret = -1;

	btrfs_blk_desc = fs_dev_desc;
	btrfs_part_info = fs_partition;

	memset(&btrfs_info, 0, sizeof(btrfs_info));

	btrfs_hash_init();
	if (btrfs_read_superblock())
		return -1;

	if (btrfs_chunk_map_init()) {
		printf("%s: failed to init chunk map\n", __func__);
		return -1;
	}

	btrfs_info.tree_root.objectid = 0;
	btrfs_info.tree_root.bytenr = btrfs_info.sb.root;
	btrfs_info.chunk_root.objectid = 0;
	btrfs_info.chunk_root.bytenr = btrfs_info.sb.chunk_root;

	if (__btrfs_read_chunk_tree()) {
		printf("%s: failed to read chunk tree\n", __func__);
		return -1;
	}

	if (btrfs_find_root(btrfs_get_default_subvol_objectid(),
			    &btrfs_info.fs_root, NULL)) {
		printf("%s: failed to find default subvolume\n", __func__);
		return -1;
	}

	fs_info = open_ctree_fs_info(fs_dev_desc, fs_partition);
	if (fs_info) {
		current_fs_info = fs_info;
		ret = 0;
	}
	return ret;
}

int btrfs_ls(const char *path)
{
	struct btrfs_fs_info *fs_info = current_fs_info;
	struct btrfs_root *root = fs_info->fs_root;
	u64 ino = BTRFS_FIRST_FREE_OBJECTID;
	u8 type;
	int ret;

	ASSERT(fs_info);
	ret = btrfs_lookup_path(fs_info->fs_root, BTRFS_FIRST_FREE_OBJECTID,
				path, &root, &ino, &type, 40);
	if (ret < 0) {
		printf("Cannot lookup path %s\n", path);
		return ret;
	}

	if (type != BTRFS_FT_DIR) {
		error("Not a directory: %s", path);
		return -ENOENT;
	}
	ret = btrfs_iter_dir(root, ino, show_dir);
	if (ret < 0) {
		error("An error occured while listing directory %s", path);
		return ret;
	}
	return 0;
}

int btrfs_exists(const char *file)
{
	struct __btrfs_root root = btrfs_info.fs_root;
	u64 inr;
	u8 type;

	inr = __btrfs_lookup_path(&root, root.root_dirid, file, &type, NULL, 40);

	return (inr != -1ULL && type == BTRFS_FT_REG_FILE);
}

int btrfs_size(const char *file, loff_t *size)
{
	struct __btrfs_root root = btrfs_info.fs_root;
	struct btrfs_inode_item inode;
	u64 inr;
	u8 type;

	inr = __btrfs_lookup_path(&root, root.root_dirid, file, &type, &inode,
				40);

	if (inr == -1ULL) {
		printf("Cannot lookup file %s\n", file);
		return -1;
	}

	if (type != BTRFS_FT_REG_FILE) {
		printf("Not a regular file: %s\n", file);
		return -1;
	}

	*size = inode.size;
	return 0;
}

int btrfs_read(const char *file, void *buf, loff_t offset, loff_t len,
	       loff_t *actread)
{
	struct __btrfs_root root = btrfs_info.fs_root;
	struct btrfs_inode_item inode;
	u64 inr, rd;
	u8 type;

	inr = __btrfs_lookup_path(&root, root.root_dirid, file, &type, &inode,
				40);

	if (inr == -1ULL) {
		printf("Cannot lookup file %s\n", file);
		return -1;
	}

	if (type != BTRFS_FT_REG_FILE) {
		printf("Not a regular file: %s\n", file);
		return -1;
	}

	if (!len)
		len = inode.size;

	if (len > inode.size - offset)
		len = inode.size - offset;

	rd = btrfs_file_read(&root, inr, offset, len, buf);
	if (rd == -1ULL) {
		printf("An error occured while reading file %s\n", file);
		return -1;
	}

	*actread = rd;
	return 0;
}

void btrfs_close(void)
{
	btrfs_chunk_map_exit();
	if (current_fs_info) {
		close_ctree_fs_info(current_fs_info);
		current_fs_info = NULL;
	}
}

int btrfs_uuid(char *uuid_str)
{
#ifdef CONFIG_LIB_UUID
	uuid_bin_to_str(btrfs_info.sb.fsid, uuid_str, UUID_STR_FORMAT_STD);
	return 0;
#endif
	return -ENOSYS;
}
