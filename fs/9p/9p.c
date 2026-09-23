// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#include <9p.h>
#include <9pfs.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <asm/unaligned.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>

static u32 p9_root_fid = 1;
static struct p9_client *current_p9_client;

int p9_fs_set_blk_dev(struct blk_desc *fs_dev_desc, struct disk_partition *fs_partition)
{
	const char *tag = NULL;
	struct udevice *dev;
	struct p9_client *c;
	int ret;

	if (fs_partition && fs_partition->name[0])
		tag = (const char *)fs_partition->name;

	ret = p9_get_device(tag, &dev);
	if (ret) {
		log_err("9P Error: No matching 9P transport device found for '%s'\n",
			tag ? tag : "-");
		return ret;
	}

	c = p9_get_client(dev);
	if (!c)
		return -ENODEV;

	if (p9_client_version(c))
		return -EIO;
	if (p9_client_attach(c, p9_root_fid, "root", ""))
		return -EIO;

	current_p9_client = c;
	return 0;
}

int p9_fs_ls(const char *dirname)
{
	struct p9_client *c = current_p9_client;
	u32 fid;
	u8 *buf, *ptr, *end;
	u32 actread;
	u64 offset = 0;
	int count = 0;
	int ret;

	if (!c)
		return -ENODEV;

	fid = p9_client_alloc_fid(c);
	if (p9_client_walk(c, p9_root_fid, fid, dirname))
		return -ENOENT;
	if (p9_client_lopen(c, fid, P9_O_RDONLY)) {
		p9_client_clunk(c, fid);
		return -EIO;
	}

	buf = malloc(4096);
	if (!buf) {
		p9_client_clunk(c, fid);
		return -ENOMEM;
	}

	while (1) {
		ret = p9_client_readdir(c, fid, offset, 4096, buf, &actread);
		if (ret || actread == 0)
			break;

		ptr = buf;
		end = buf + actread;
		while (ptr + sizeof(struct p9_dirent_hdr) <= end) {
			struct p9_dirent_hdr *entry = (struct p9_dirent_hdr *)ptr;
			u64 next_offset = get_unaligned_le64(&entry->offset);
			u8 d_type = entry->type;
			u16 name_len = get_unaligned_le16(&entry->name_len);
			const char *name = (const char *)(entry + 1);

			if (ptr + sizeof(struct p9_dirent_hdr) + name_len > end)
				break;
			printf("  %s %.*s\n", (d_type == P9_DT_DIR) ? "<DIR> " : "      ",
			       name_len, name);
			offset = next_offset;
			ptr += sizeof(struct p9_dirent_hdr) + name_len;
			count++;
		}
	}

	free(buf);
	p9_client_clunk(c, fid);

	if (ret)
		return ret;

	if (count == 0)
		printf("  (empty directory)\n");

	return 0;
}

int p9_fs_size(const char *filename, loff_t *size)
{
	struct p9_client *c = current_p9_client;
	u32 fid;
	int ret;

	if (!c)
		return -ENODEV;

	fid = p9_client_alloc_fid(c);
	if (p9_client_walk(c, p9_root_fid, fid, filename))
		return -ENOENT;
	ret = p9_client_stat(c, fid, size);
	p9_client_clunk(c, fid);

	return ret;
}

int p9_fs_exists(const char *filename)
{
	loff_t size;

	return (p9_fs_size(filename, &size) == 0) ? 1 : 0;
}

int p9_fs_read(const char *filename, void *buf, loff_t offset, loff_t len, loff_t *actread)
{
	struct p9_client *c = current_p9_client;
	u32 fid;
	u32 read_bytes = 0;
	loff_t total_read = 0;
	loff_t remaining;
	u32 req_len;
	u8 *dst = (u8 *)buf;
	int ret = 0;

	if (!c)
		return -ENODEV;

	fid = p9_client_alloc_fid(c);
	if (p9_client_walk(c, p9_root_fid, fid, filename))
		return -ENOENT;
	if (p9_client_lopen(c, fid, P9_O_RDONLY)) {
		p9_client_clunk(c, fid);
		return -EIO;
	}

	while (1) {
		if (len == 0) {
			req_len = 0;
		} else {
			remaining = len - total_read;
			if (remaining == 0)
				break;

			req_len = (remaining > 0xFFFFFFFF) ? 0xFFFFFFFF : (u32)remaining;
		}

		ret = p9_client_read(c, fid, offset, req_len, dst, &read_bytes);

		if (ret != 0 || read_bytes == 0)
			break;

		total_read += read_bytes;
		offset += read_bytes;
		dst += read_bytes;
	}

	*actread = total_read;
	p9_client_clunk(c, fid);

	return ret ? ret : 0;
}
