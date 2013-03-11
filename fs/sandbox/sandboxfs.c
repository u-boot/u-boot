/*
 * Copyright (c) 2012, Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
