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
#define FS_TYPE_SANDBOX	3

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
 * Determine whether a file exists
 *
 * Returns 1 if the file exists, 0 if it doesn't exist.
 */
int fs_exists(const char *filename);

/*
 * Determine a file's size
 *
 * Returns the file's size in bytes, or a negative value if it doesn't exist.
 */
int fs_size(const char *filename);

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
 * Write file "filename" to the partition previously set by fs_set_blk_dev(),
 * from address "addr", starting at byte offset "offset", and writing "len"
 * bytes. "offset" may be 0 to write to the start of the file. Note that not
 * all filesystem types support offset!=0.
 *
 * Returns number of bytes read on success. Returns <= 0 on error.
 */
int fs_write(const char *filename, ulong addr, int offset, int len);

/*
 * Common implementation for various filesystem commands, optionally limited
 * to a specific filesystem type via the fstype parameter.
 */
int do_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int file_exists(const char *dev_type, const char *dev_part, const char *file,
		int fstype);
int do_save(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);

#endif /* _FS_H */
