/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _FS_H
#define _FS_H

#include <common.h>

#define FS_TYPE_ANY	0
#define FS_TYPE_FAT	1
#define FS_TYPE_EXT	2

/*
 * Tell the fs layer which block device an partition to use for future
 * commands. This also internally identifies the filesystem that is present
 * within the partition. The identification process may be limited to a
 * specific filesystem type by passing FS_* in the fstype parameter.
 *
 * Returns 0 on success.
 * Returns non-zero if there is an error accessing the disk or partition, or
 * no known filesystem type could be recognized on it.
 */
int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype);

/*
 * Print the list of files on the partition previously set by fs_set_blk_dev(),
 * in directory "dirname".
 *
 * Returns 0 on success. Returns non-zero on error.
 */
int fs_ls(const char *dirname);

/*
 * Read file "filename" from the partition previously set by fs_set_blk_dev(),
 * to address "addr", starting at byte offset "offset", and reading "len"
 * bytes. "offset" may be 0 to read from the start of the file. "len" may be
 * 0 to read the entire file. Note that not all filesystem types support
 * either/both offset!=0 or len!=0.
 *
 * Returns number of bytes read on success. Returns <= 0 on error.
 */
int fs_read(const char *filename, ulong addr, int offset, int len);

/*
 * Common implementation for various filesystem commands, optionally limited
 * to a specific filesystem type via the fstype parameter.
 */
int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype, int cmdline_base);
int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);

#endif /* _FS_H */
