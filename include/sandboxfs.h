/*
 * Copyright (c) 2012, Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __SANDBOX_FS__
#define __SANDBOX_FS__

int sandbox_fs_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info);

long sandbox_fs_read_at(const char *filename, unsigned long pos,
			     void *buffer, unsigned long maxsize);

void sandbox_fs_close(void);
int sandbox_fs_ls(const char *dirname);
int sandbox_fs_exists(const char *filename);
int fs_read_sandbox(const char *filename, void *buf, int offset, int len);
int fs_write_sandbox(const char *filename, void *buf, int offset, int len);

#endif
