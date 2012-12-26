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

#include <config.h>
#include <common.h>
#include <part.h>
#include <ext4fs.h>
#include <fat.h>
#include <fs.h>

DECLARE_GLOBAL_DATA_PTR;

static block_dev_desc_t *fs_dev_desc;
static disk_partition_t fs_partition;
static int fs_type = FS_TYPE_ANY;

static inline int fs_probe_unsupported(block_dev_desc_t *fs_dev_desc,
				      disk_partition_t *fs_partition)
{
	printf("** Unrecognized filesystem type **\n");
	return -1;
}

static inline int fs_ls_unsupported(const char *dirname)
{
	return -1;
}

static inline int fs_read_unsupported(const char *filename, ulong addr,
				      int offset, int len)
{
	return -1;
}

static inline void fs_close_unsupported(void)
{
}

#ifdef CONFIG_FS_FAT
static int fs_probe_fat(block_dev_desc_t *fs_dev_desc,
			disk_partition_t *fs_partition)
{
	return fat_set_blk_dev(fs_dev_desc, fs_partition);
}

static void fs_close_fat(void)
{
}

#define fs_ls_fat file_fat_ls

static int fs_read_fat(const char *filename, ulong addr, int offset, int len)
{
	int len_read;

	len_read = file_fat_read_at(filename, offset,
				    (unsigned char *)addr, len);
	if (len_read == -1) {
		printf("** Unable to read file %s **\n", filename);
		return -1;
	}

	return len_read;
}
#else
static inline int fs_probe_fat(void)
{
	return -1;
}

static inline void fs_close_fat(void)
{
}

#define fs_ls_fat fs_ls_unsupported
#define fs_read_fat fs_read_unsupported
#endif

#ifdef CONFIG_FS_EXT4
static int fs_probe_ext(block_dev_desc_t *fs_dev_desc,
			disk_partition_t *fs_partition)
{
	ext4fs_set_blk_dev(fs_dev_desc, fs_partition);

	if (!ext4fs_mount(fs_partition->size)) {
		ext4fs_close();
		return -1;
	}

	return 0;
}

static void fs_close_ext(void)
{
	ext4fs_close();
}

#define fs_ls_ext ext4fs_ls

static int fs_read_ext(const char *filename, ulong addr, int offset, int len)
{
	int file_len;
	int len_read;

	if (offset != 0) {
		printf("** Cannot support non-zero offset **\n");
		return -1;
	}

	file_len = ext4fs_open(filename);
	if (file_len < 0) {
		printf("** File not found %s **\n", filename);
		ext4fs_close();
		return -1;
	}

	if (len == 0)
		len = file_len;

	len_read = ext4fs_read((char *)addr, len);
	ext4fs_close();

	if (len_read != len) {
		printf("** Unable to read file %s **\n", filename);
		return -1;
	}

	return len_read;
}
#else
static inline int fs_probe_ext(void)
{
	return -1;
}

static inline void fs_close_ext(void)
{
}

#define fs_ls_ext fs_ls_unsupported
#define fs_read_ext fs_read_unsupported
#endif

struct fstype_info {
	int fstype;
	int (*probe)(block_dev_desc_t *fs_dev_desc,
		     disk_partition_t *fs_partition);
	int (*ls)(const char *dirname);
	int (*read)(const char *filename, ulong addr, int offset, int len);
	void (*close)(void);
};

static struct fstype_info fstypes[] = {
#ifdef CONFIG_FS_FAT
	{
		.fstype = FS_TYPE_FAT,
		.probe = fs_probe_fat,
		.close = fs_close_fat,
		.ls = file_fat_ls,
		.read = fs_read_fat,
	},
#endif
#ifdef CONFIG_FS_EXT4
	{
		.fstype = FS_TYPE_EXT,
		.probe = fs_probe_ext,
		.close = fs_close_ext,
		.ls = ext4fs_ls,
		.read = fs_read_ext,
	},
#endif
	{
		.fstype = FS_TYPE_ANY,
		.probe = fs_probe_unsupported,
		.close = fs_close_unsupported,
		.ls = fs_ls_unsupported,
		.read = fs_read_unsupported,
	},
};

static struct fstype_info *fs_get_info(int fstype)
{
	struct fstype_info *info;
	int i;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes) - 1; i++, info++) {
		if (fstype == info->fstype)
			return info;
	}

	/* Return the 'unsupported' sentinel */
	return info;
}

int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype)
{
	struct fstype_info *info;
	int part, i;
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	static int relocated;

	if (!relocated) {
		for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes);
				i++, info++) {
			info->probe += gd->reloc_off;
			info->close += gd->reloc_off;
			info->ls += gd->reloc_off;
			info->read += gd->reloc_off;
		}
		relocated = 1;
	}
#endif

	part = get_device_and_partition(ifname, dev_part_str, &fs_dev_desc,
					&fs_partition, 1);
	if (part < 0)
		return -1;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (fstype != FS_TYPE_ANY && info->fstype != FS_TYPE_ANY &&
				fstype != info->fstype)
			continue;

		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			return 0;
		}
	}

	return -1;
}

static void fs_close(void)
{
	struct fstype_info *info = fs_get_info(fs_type);

	info->close();
	fs_type = FS_TYPE_ANY;
}

int fs_ls(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->ls(dirname);

	fs_close();

	return ret;
}

int fs_read(const char *filename, ulong addr, int offset, int len)
{
	struct fstype_info *info = fs_get_info(fs_type);
	int ret;

	ret = info->read(filename, addr, offset, len);

	/* If we requested a specific number of bytes, check we got it */
	if (ret >= 0 && len && ret != len) {
		printf("** Unable to read file %s **\n", filename);
		ret = -1;
	}
	fs_close();

	return ret;
}

int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype, int cmdline_base)
{
	unsigned long addr;
	const char *addr_str;
	const char *filename;
	unsigned long bytes;
	unsigned long pos;
	int len_read;
	unsigned long time;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 7)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (argc >= 4) {
		addr = simple_strtoul(argv[3], NULL, cmdline_base);
	} else {
		addr_str = getenv("loadaddr");
		if (addr_str != NULL)
			addr = simple_strtoul(addr_str, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
	}
	if (argc >= 5) {
		filename = argv[4];
	} else {
		filename = getenv("bootfile");
		if (!filename) {
			puts("** No boot file defined **\n");
			return 1;
		}
	}
	if (argc >= 6)
		bytes = simple_strtoul(argv[5], NULL, cmdline_base);
	else
		bytes = 0;
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, cmdline_base);
	else
		pos = 0;

	time = get_timer(0);
	len_read = fs_read(filename, addr, pos, bytes);
	time = get_timer(time);
	if (len_read <= 0)
		return 1;

	printf("%d bytes read in %lu ms", len_read, time);
	if (time > 0) {
		puts(" (");
		print_size(len_read / time * 1000, "/s");
		puts(")");
	}
	puts("\n");

	setenv_hex("filesize", len_read);

	return 0;
}

int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	int fstype)
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (fs_ls(argc >= 4 ? argv[3] : "/"))
		return 1;

	return 0;
}
