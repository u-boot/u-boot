/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#ifndef __9PFS_H__
#define __9PFS_H__

#include <linux/types.h>

struct blk_desc;
struct disk_partition;

/**
 * p9_fs_set_blk_dev() - Set/probe block device for 9P filesystem
 * @fs_dev_desc: Block device descriptor (expected NULL for pseudo dev)
 * @fs_partition: Disk partition information
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_fs_set_blk_dev(struct blk_desc *fs_dev_desc, struct disk_partition *fs_partition);

/**
 * p9_fs_ls() - List files in a directory
 * @dirname: Path to directory
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_fs_ls(const char *dirname);

/**
 * p9_fs_exists() - Check if a file exists
 * @filename: Path to file
 *
 * Return: 1 if file exists, 0 otherwise
 */
int p9_fs_exists(const char *filename);

/**
 * p9_fs_size() - Get size of a file
 * @filename: Path to file
 * @size: Output pointer for size
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_fs_size(const char *filename, loff_t *size);

/**
 * p9_fs_read() - Read file content into memory
 * @filename: Path to file
 * @buf: Destination memory buffer
 * @offset: Byte offset within file
 * @len: Maximum bytes to read (0 for entire file)
 * @actread: Output pointer for actual bytes read
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_fs_read(const char *filename, void *buf, loff_t offset, loff_t len, loff_t *actread);

#endif /* __9PFS_H__ */
