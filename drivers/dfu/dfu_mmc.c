/*
 * dfu.c -- DFU back-end routines
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <div64.h>
#include <dfu.h>
#include <mmc.h>

static unsigned char __aligned(CONFIG_SYS_CACHELINE_SIZE)
				dfu_file_buf[CONFIG_SYS_DFU_MAX_FILE_SIZE];
static long dfu_file_buf_len;

static int mmc_block_op(enum dfu_op op, struct dfu_entity *dfu,
			u64 offset, void *buf, long *len)
{
	struct mmc *mmc = find_mmc_device(dfu->dev_num);
	u32 blk_start, blk_count, n = 0;

	/*
	 * We must ensure that we work in lba_blk_size chunks, so ALIGN
	 * this value.
	 */
	*len = ALIGN(*len, dfu->data.mmc.lba_blk_size);

	blk_start = dfu->data.mmc.lba_start +
			(u32)lldiv(offset, dfu->data.mmc.lba_blk_size);
	blk_count = *len / dfu->data.mmc.lba_blk_size;
	if (blk_start + blk_count >
			dfu->data.mmc.lba_start + dfu->data.mmc.lba_size) {
		puts("Request would exceed designated area!\n");
		return -EINVAL;
	}

	debug("%s: %s dev: %d start: %d cnt: %d buf: 0x%p\n", __func__,
	      op == DFU_OP_READ ? "MMC READ" : "MMC WRITE", dfu->dev_num,
	      blk_start, blk_count, buf);
	switch (op) {
	case DFU_OP_READ:
		n = mmc->block_dev.block_read(dfu->dev_num, blk_start,
					      blk_count, buf);
		break;
	case DFU_OP_WRITE:
		n = mmc->block_dev.block_write(dfu->dev_num, blk_start,
					       blk_count, buf);
		break;
	default:
		error("Operation not supported\n");
	}

	if (n != blk_count) {
		error("MMC operation failed");
		return -EIO;
	}

	return 0;
}

static int mmc_file_buffer(struct dfu_entity *dfu, void *buf, long *len)
{
	if (dfu_file_buf_len + *len > CONFIG_SYS_DFU_MAX_FILE_SIZE) {
		dfu_file_buf_len = 0;
		return -EINVAL;
	}

	/* Add to the current buffer. */
	memcpy(dfu_file_buf + dfu_file_buf_len, buf, *len);
	dfu_file_buf_len += *len;

	return 0;
}

static int mmc_file_op(enum dfu_op op, struct dfu_entity *dfu,
			void *buf, long *len)
{
	char cmd_buf[DFU_CMD_BUF_SIZE];
	char *str_env;
	int ret;

	switch (dfu->layout) {
	case DFU_FS_FAT:
		sprintf(cmd_buf, "fat%s mmc %d:%d 0x%x %s",
			op == DFU_OP_READ ? "load" : "write",
			dfu->data.mmc.dev, dfu->data.mmc.part,
			(unsigned int) buf, dfu->name);
		break;
	case DFU_FS_EXT4:
		sprintf(cmd_buf, "ext4%s mmc %d:%d 0x%x /%s",
			op == DFU_OP_READ ? "load" : "write",
			dfu->data.mmc.dev, dfu->data.mmc.part,
			(unsigned int) buf, dfu->name);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
		return -1;
	}

	if (op == DFU_OP_WRITE)
		sprintf(cmd_buf + strlen(cmd_buf), " %lx", *len);

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

int dfu_write_medium_mmc(struct dfu_entity *dfu,
		u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_buffer(dfu, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_flush_medium_mmc(struct dfu_entity *dfu)
{
	int ret = 0;

	if (dfu->layout != DFU_RAW_ADDR) {
		/* Do stuff here. */
		ret = mmc_file_op(DFU_OP_WRITE, dfu, &dfu_file_buf,
				&dfu_file_buf_len);

		/* Now that we're done */
		dfu_file_buf_len = 0;
	}

	return ret;
}

int dfu_read_medium_mmc(struct dfu_entity *dfu, u64 offset, void *buf,
		long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_op(DFU_OP_READ, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_op(DFU_OP_READ, dfu, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

/*
 * @param s Parameter string containing space-separated arguments:
 *	1st:
 *		raw	(raw read/write)
 *		fat	(files)
 *		ext4	(^)
 *		part	(partition image)
 *	2nd and 3rd:
 *		lba_start and lba_size, for raw write
 *		mmc_dev and mmc_part, for filesystems and part
 */
int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *s)
{
	const char *entity_type;
	size_t second_arg;
	size_t third_arg;

	struct mmc *mmc;

	const char *argv[3];
	const char **parg = argv;

	for (; parg < argv + sizeof(argv) / sizeof(*argv); ++parg) {
		*parg = strsep(&s, " ");
		if (*parg == NULL) {
			error("Invalid number of arguments.\n");
			return -ENODEV;
		}
	}

	entity_type = argv[0];
	/*
	 * Base 0 means we'll accept (prefixed with 0x or 0) base 16, 8,
	 * with default 10.
	 */
	second_arg = simple_strtoul(argv[1], NULL, 0);
	third_arg = simple_strtoul(argv[2], NULL, 0);

	mmc = find_mmc_device(dfu->dev_num);
	if (mmc == NULL) {
		error("Couldn't find MMC device no. %d.\n", dfu->dev_num);
		return -ENODEV;
	}

	if (mmc_init(mmc)) {
		error("Couldn't init MMC device.\n");
		return -ENODEV;
	}

	if (!strcmp(entity_type, "raw")) {
		dfu->layout			= DFU_RAW_ADDR;
		dfu->data.mmc.lba_start		= second_arg;
		dfu->data.mmc.lba_size		= third_arg;
		dfu->data.mmc.lba_blk_size	= mmc->read_bl_len;
	} else if (!strcmp(entity_type, "part")) {
		disk_partition_t partinfo;
		block_dev_desc_t *blk_dev = &mmc->block_dev;
		int mmcdev = second_arg;
		int mmcpart = third_arg;

		if (get_partition_info(blk_dev, mmcpart, &partinfo) != 0) {
			error("Couldn't find part #%d on mmc device #%d\n",
			      mmcpart, mmcdev);
			return -ENODEV;
		}

		dfu->layout			= DFU_RAW_ADDR;
		dfu->data.mmc.lba_start		= partinfo.start;
		dfu->data.mmc.lba_size		= partinfo.size;
		dfu->data.mmc.lba_blk_size	= partinfo.blksz;
	} else if (!strcmp(entity_type, "fat")) {
		dfu->layout = DFU_FS_FAT;
	} else if (!strcmp(entity_type, "ext4")) {
		dfu->layout = DFU_FS_EXT4;
	} else {
		error("Memory layout (%s) not supported!\n", entity_type);
		return -ENODEV;
	}

	/* if it's NOT a raw write */
	if (strcmp(entity_type, "raw")) {
		dfu->data.mmc.dev = second_arg;
		dfu->data.mmc.part = third_arg;
	}

	dfu->dev_type = DFU_DEV_MMC;
	dfu->read_medium = dfu_read_medium_mmc;
	dfu->write_medium = dfu_write_medium_mmc;
	dfu->flush_medium = dfu_flush_medium_mmc;
	dfu->inited = 0;

	return 0;
}
