/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _EXFAT_H_
#define _EXFAT_H_

struct disk_partition;

int exfat_fs_opendir(const char *filename, struct fs_dir_stream **dirsp);
int exfat_fs_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
int exfat_fs_ls(const char *dirname);
int exfat_fs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition);
int exfat_fs_read(const char *filename, void *buf, loff_t offset,
	       loff_t len, loff_t *actread);
int exfat_fs_size(const char *filename, loff_t *size);
int exfat_fs_exists(const char *filename);
void exfat_fs_close(void);
void exfat_fs_closedir(struct fs_dir_stream *dirs);

int exfat_fs_unlink(const char *filename);
int exfat_fs_mkdir(const char *dirname);
int exfat_fs_write(const char *filename, void *buf, loff_t offset,
		   loff_t len, loff_t *actwrite);
int exfat_fs_rename(const char *old_path, const char *new_path);

#endif /* _EXFAT_H */
