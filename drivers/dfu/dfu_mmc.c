/*
 * dfu.c -- DFU back-end routines
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <malloc.h>
#include <dfu.h>

enum dfu_mmc_op {
	DFU_OP_READ = 1,
	DFU_OP_WRITE,
};

static int mmc_block_op(enum dfu_mmc_op op, struct dfu_entity *dfu,
			void *buf, long *len)
{
	char cmd_buf[DFU_CMD_BUF_SIZE];

	sprintf(cmd_buf, "mmc %s 0x%x %x %x",
		op == DFU_OP_READ ? "read" : "write",
		(unsigned int) buf,
		dfu->data.mmc.lba_start,
		dfu->data.mmc.lba_size);

	if (op == DFU_OP_READ)
		*len = dfu->data.mmc.lba_blk_size * dfu->data.mmc.lba_size;

	debug("%s: %s 0x%p\n", __func__, cmd_buf, cmd_buf);
	return run_command(cmd_buf, 0);
}

static inline int mmc_block_write(struct dfu_entity *dfu, void *buf, long *len)
{
	return mmc_block_op(DFU_OP_WRITE, dfu, buf, len);
}

static inline int mmc_block_read(struct dfu_entity *dfu, void *buf, long *len)
{
	return mmc_block_op(DFU_OP_READ, dfu, buf, len);
}

static int mmc_file_op(enum dfu_mmc_op op, struct dfu_entity *dfu,
			void *buf, long *len)
{
	char cmd_buf[DFU_CMD_BUF_SIZE];
	char *str_env;
	int ret;

	switch (dfu->layout) {
	case DFU_FS_FAT:
		sprintf(cmd_buf, "fat%s mmc %d:%d 0x%x %s %lx",
			op == DFU_OP_READ ? "load" : "write",
			dfu->data.mmc.dev, dfu->data.mmc.part,
			(unsigned int) buf, dfu->name, *len);
		break;
	case DFU_FS_EXT4:
		sprintf(cmd_buf, "ext4%s mmc %d:%d /%s 0x%x %ld",
			op == DFU_OP_READ ? "load" : "write",
			dfu->data.mmc.dev, dfu->data.mmc.part,
			dfu->name, (unsigned int) buf, *len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	debug("%s: %s 0x%p\n", __func__, cmd_buf, cmd_buf);

	ret = run_command(cmd_buf, 0);
	if (ret) {
		puts("dfu: Read error!\n");
		return ret;
	}

	if (dfu->layout != DFU_RAW_ADDR && op == DFU_OP_READ) {
		str_env = getenv("filesize");
		if (str_env == NULL) {
			puts("dfu: Wrong file size!\n");
			return -1;
		}
		*len = simple_strtoul(str_env, NULL, 16);
	}

	return ret;
}

static inline int mmc_file_write(struct dfu_entity *dfu, void *buf, long *len)
{
	return mmc_file_op(DFU_OP_WRITE, dfu, buf, len);
}

static inline int mmc_file_read(struct dfu_entity *dfu, void *buf, long *len)
{
	return mmc_file_op(DFU_OP_READ, dfu, buf, len);
}

int dfu_write_medium_mmc(struct dfu_entity *dfu, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_write(dfu, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_write(dfu, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_read_medium_mmc(struct dfu_entity *dfu, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_read(dfu, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_read(dfu, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *s)
{
	char *st;

	dfu->dev_type = DFU_DEV_MMC;
	st = strsep(&s, " ");
	if (!strcmp(st, "mmc")) {
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.mmc.lba_start = simple_strtoul(s, &s, 16);
		dfu->data.mmc.lba_size = simple_strtoul(++s, &s, 16);
		dfu->data.mmc.lba_blk_size = get_mmc_blk_size(dfu->dev_num);
	} else if (!strcmp(st, "fat")) {
		dfu->layout = DFU_FS_FAT;
	} else if (!strcmp(st, "ext4")) {
		dfu->layout = DFU_FS_EXT4;
	} else {
		printf("%s: Memory layout (%s) not supported!\n", __func__, st);
	}

	if (dfu->layout == DFU_FS_EXT4 || dfu->layout == DFU_FS_FAT) {
		dfu->data.mmc.dev = simple_strtoul(s, &s, 10);
		dfu->data.mmc.part = simple_strtoul(++s, &s, 10);
	}

	dfu->read_medium = dfu_read_medium_mmc;
	dfu->write_medium = dfu_write_medium_mmc;

	return 0;
}
