/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#ifndef __U_BOOT_BTRFS_H__
#define __U_BOOT_BTRFS_H__

struct blk_desc;
struct disk_partition;
struct fs_dir_stream;
struct fs_dirent;

int btrfs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition);
int btrfs_exists(const char *);
int btrfs_size(const char *, loff_t *);
int btrfs_read(const char *, void *, loff_t, loff_t, loff_t *);
void btrfs_close(void);
int btrfs_uuid(char *);
void btrfs_list_subvols(void);
int btrfs_opendir(const char *filename, struct fs_dir_stream **dirsp);
int btrfs_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
void btrfs_closedir(struct fs_dir_stream *dirs);

#endif /* __U_BOOT_BTRFS_H__ */
