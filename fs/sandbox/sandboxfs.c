/*
 * Copyright (c) 2012, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fs.h>
#include <os.h>

int sandbox_fs_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info)
{
	return 0;
}

long sandbox_fs_read_at(const char *filename, unsigned long pos,
			     void *buffer, unsigned long maxsize)
{
	ssize_t size;
	int fd, ret;

	fd = os_open(filename, OS_O_RDONLY);
	if (fd < 0)
		return fd;
	ret = os_lseek(fd, pos, OS_SEEK_SET);
	if (ret == -1) {
		os_close(fd);
		return ret;
	}
	if (!maxsize)
		maxsize = os_get_filesize(filename);
	size = os_read(fd, buffer, maxsize);
	os_close(fd);

	return size;
}

long sandbox_fs_write_at(const char *filename, unsigned long pos,
			 void *buffer, unsigned long towrite)
{
	ssize_t size;
	int fd, ret;

	fd = os_open(filename, OS_O_RDWR | OS_O_CREAT);
	if (fd < 0)
		return fd;
	ret = os_lseek(fd, pos, OS_SEEK_SET);
	if (ret == -1) {
		os_close(fd);
		return ret;
	}
	size = os_write(fd, buffer, towrite);
	os_close(fd);

	return size;
}

int sandbox_fs_ls(const char *dirname)
{
	struct os_dirent_node *head, *node;
	int ret;

	ret = os_dirent_ls(dirname, &head);
	if (ret)
		return ret;

	for (node = head; node; node = node->next) {
		printf("%s %10lu %s\n", os_dirent_get_typename(node->type),
		       node->size, node->name);
	}

	return 0;
}

int sandbox_fs_exists(const char *filename)
{
	ssize_t sz;

	sz = os_get_filesize(filename);
	return sz >= 0;
}

void sandbox_fs_close(void)
{
}

int fs_read_sandbox(const char *filename, void *buf, int offset, int len)
{
	int len_read;

	len_read = sandbox_fs_read_at(filename, offset, buf, len);
	if (len_read == -1) {
		printf("** Unable to read file %s **\n", filename);
		return -1;
	}

	return len_read;
}

int fs_write_sandbox(const char *filename, void *buf, int offset, int len)
{
	int len_written;

	len_written = sandbox_fs_write_at(filename, offset, buf, len);
	if (len_written == -1) {
		printf("** Unable to write file %s **\n", filename);
		return -1;
	}

	return len_written;
}
