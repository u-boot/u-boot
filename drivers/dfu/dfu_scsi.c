// SPDX-License-Identifier: GPL-2.0+
/*
 * DFU SCSI backend (based on MMC backend).
 *
 * Copyright (C) 2012 Samsung Electronics
 *   author: Lukasz Majewski <l.majewski@samsung.com>
 * Copyright (C) 2024 Linaro Ltd.
 */

#include <log.h>
#include <malloc.h>
#include <errno.h>
#include <div64.h>
#include <dfu.h>
#include <ext4fs.h>
#include <fat.h>
#include <scsi.h>
#include <part.h>
#include <command.h>
#include <linux/printk.h>

static unsigned char *dfu_file_buf;
static u64 dfu_file_buf_len;
static u64 dfu_file_buf_offset;

#define scsi_get_blk_desc(dev) ((struct blk_desc *)dev_get_uclass_plat(dev))

#define find_scsi_device(dev_num, scsi) blk_get_device(UCLASS_SCSI, dev_num, scsi)

static int scsi_block_op(enum dfu_op op, struct dfu_entity *dfu, u64 offset, void *buf, long *len)
{
	struct udevice *scsi;
	u32 blk_start, blk_count, n = 0;
	int ret;

	ret = find_scsi_device(dfu->data.scsi.lun, &scsi);
	if (ret < 0) {
		pr_err("Device scsi %d - not found!", dfu->data.scsi.lun);
		return -ENODEV;
	}

	/*
	 * We must ensure that we work in lba_blk_size chunks, so ALIGN
	 * this value.
	 */
	*len = ALIGN(*len, dfu->data.scsi.lba_blk_size);

	blk_start = dfu->data.scsi.lba_start + (u32)lldiv(offset, dfu->data.scsi.lba_blk_size);
	blk_count = *len / dfu->data.scsi.lba_blk_size;
	if (blk_start + blk_count > dfu->data.scsi.lba_start + dfu->data.scsi.lba_size) {
		puts("Request would exceed designated area!\n");
		return -EINVAL;
	}

	debug("%s: %s dev: %d start: %d cnt: %d buf: 0x%p\n", __func__,
	      op == DFU_OP_READ ? "scsi READ" : "scsi WRITE", dfu->data.scsi.lun, blk_start,
	      blk_count, buf);
	switch (op) {
	case DFU_OP_READ:
		n = blk_dread(scsi_get_blk_desc(scsi), blk_start, blk_count, buf);
		break;
	case DFU_OP_WRITE:
		n = blk_dwrite(scsi_get_blk_desc(scsi), blk_start, blk_count, buf);
		break;
	default:
		pr_err("Operation not supported\n");
	}

	if (n != blk_count) {
		pr_err("scsi block operation failed");
		return -EIO;
	}

	return 0;
}

static int scsi_file_op(enum dfu_op op, struct dfu_entity *dfu, u64 offset, void *buf, u64 *len)
{
	char dev_part_str[8];
	int ret;
	int fstype;
	loff_t size = 0;

	switch (dfu->layout) {
	case DFU_FS_FAT:
		fstype = FS_TYPE_FAT;
		break;
	case DFU_FS_EXT4:
		fstype = FS_TYPE_EXT;
		break;
	case DFU_SKIP:
		return 0;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
		return -1;
	}

	snprintf(dev_part_str, sizeof(dev_part_str), "%d:%d", dfu->data.scsi.dev,
		 dfu->data.scsi.part);

	ret = fs_set_blk_dev("scsi", dev_part_str, fstype);
	if (ret) {
		puts("dfu: fs_set_blk_dev error!\n");
		return ret;
	}

	switch (op) {
	case DFU_OP_READ:
		ret = fs_read(dfu->name, (size_t)buf, offset, *len, &size);
		if (ret) {
			puts("dfu: fs_read error!\n");
			return ret;
		}
		*len = size;
		break;
	case DFU_OP_WRITE:
		ret = fs_write(dfu->name, (size_t)buf, offset, *len, &size);
		if (ret) {
			puts("dfu: fs_write error!\n");
			return ret;
		}
		break;
	case DFU_OP_SIZE:
		ret = fs_size(dfu->name, &size);
		if (ret) {
			puts("dfu: fs_size error!\n");
			return ret;
		}
		*len = size;
		break;
	default:
		return -1;
	}

	return ret;
}

static int scsi_file_buf_write(struct dfu_entity *dfu, u64 offset, void *buf, long *len)
{
	int ret = 0;

	if (offset == 0) {
		dfu_file_buf_len = 0;
		dfu_file_buf_offset = 0;
	}

	/* Add to the current buffer. */
	if (dfu_file_buf_len + *len > CONFIG_SYS_DFU_MAX_FILE_SIZE)
		*len = CONFIG_SYS_DFU_MAX_FILE_SIZE - dfu_file_buf_len;
	memcpy(dfu_file_buf + dfu_file_buf_len, buf, *len);
	dfu_file_buf_len += *len;

	if (dfu_file_buf_len == CONFIG_SYS_DFU_MAX_FILE_SIZE) {
		ret = scsi_file_op(DFU_OP_WRITE, dfu, dfu_file_buf_offset, dfu_file_buf,
				   &dfu_file_buf_len);
		dfu_file_buf_offset += dfu_file_buf_len;
		dfu_file_buf_len = 0;
	}

	return ret;
}

static int scsi_file_buf_write_finish(struct dfu_entity *dfu)
{
	int ret = scsi_file_op(DFU_OP_WRITE, dfu, dfu_file_buf_offset, dfu_file_buf,
			       &dfu_file_buf_len);

	/* Now that we're done */
	dfu_file_buf_len = 0;
	dfu_file_buf_offset = 0;

	return ret;
}

int dfu_write_medium_scsi(struct dfu_entity *dfu, u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = scsi_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = scsi_file_buf_write(dfu, offset, buf, len);
		break;
	case DFU_SCRIPT:
		ret = run_command_list(buf, *len, 0);
		break;
	case DFU_SKIP:
		ret = 0;
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_flush_medium_scsi(struct dfu_entity *dfu)
{
	int ret = 0;

	switch (dfu->layout) {
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = scsi_file_buf_write_finish(dfu);
		break;
	case DFU_SCRIPT:
		/* script may have changed the dfu_alt_info */
		dfu_reinit_needed = true;
		break;
	case DFU_RAW_ADDR:
	case DFU_SKIP:
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_get_medium_size_scsi(struct dfu_entity *dfu, u64 *size)
{
	int ret;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		*size = dfu->data.scsi.lba_size * dfu->data.scsi.lba_blk_size;
		return 0;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = scsi_file_op(DFU_OP_SIZE, dfu, 0, NULL, size);
		if (ret < 0)
			return ret;
		return 0;
	case DFU_SCRIPT:
	case DFU_SKIP:
		return 0;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
		return -1;
	}
}

static int scsi_file_buf_read(struct dfu_entity *dfu, u64 offset, void *buf, long *len)
{
	int ret;

	if (offset == 0 || offset >= dfu_file_buf_offset + dfu_file_buf_len ||
	    offset + *len < dfu_file_buf_offset) {
		u64 file_len = CONFIG_SYS_DFU_MAX_FILE_SIZE;

		ret = scsi_file_op(DFU_OP_READ, dfu, offset, dfu_file_buf, &file_len);
		if (ret < 0)
			return ret;
		dfu_file_buf_len = file_len;
		dfu_file_buf_offset = offset;
	}
	if (offset + *len > dfu_file_buf_offset + dfu_file_buf_len)
		return -EINVAL;

	/* Add to the current buffer. */
	memcpy(buf, dfu_file_buf + offset - dfu_file_buf_offset, *len);

	return 0;
}

int dfu_read_medium_scsi(struct dfu_entity *dfu, u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = scsi_block_op(DFU_OP_READ, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = scsi_file_buf_read(dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

void dfu_free_entity_scsi(struct dfu_entity *dfu)
{
	if (dfu_file_buf) {
		free(dfu_file_buf);
		dfu_file_buf = NULL;
	}
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
 *		scsi_dev and scsi_part, for filesystems and part
 */
int dfu_fill_entity_scsi(struct dfu_entity *dfu, char *devstr, char **argv, int argc)
{
	const char *entity_type;
	ssize_t second_arg;
	ssize_t third_arg = -1;
	struct udevice *scsi;
	struct blk_desc *blk_dev;
	int ret;
	char *s;

	if (argc < 2) {
		pr_err("Need at least one argument\n");
		return -EINVAL;
	}

	dfu->data.scsi.lun = dectoul(devstr, &s);
	if (*s)
		return -EINVAL;

	entity_type = argv[0];
	/*
	 * Base 0 means we'll accept (prefixed with 0x or 0) base 16, 8,
	 * with default 10.
	 */
	second_arg = simple_strtol(argv[1], &s, 0);
	if (*s)
		return -EINVAL;
	if (argc >= 3) {
		third_arg = simple_strtoul(argv[2], &s, 0);
		if (*s)
			return -EINVAL;
	}

	if (scsi_scan(false)) {
		pr_err("Couldn't init scsi device.\n");
		return -ENODEV;
	}

	ret = find_scsi_device(dfu->data.scsi.lun, &scsi);
	if (ret < 0) {
		pr_err("Couldn't find scsi device no. %d.\n", dfu->data.scsi.lun);
		return -ENODEV;
	}

	blk_dev = scsi_get_blk_desc(scsi);
	if (!blk_dev) {
		pr_err("Couldn't get block device for scsi device no. %d.\n", dfu->data.scsi.lun);
		return -ENODEV;
	}

	/* if it's NOT a raw write */
	if (strcmp(entity_type, "raw")) {
		dfu->data.scsi.dev = (second_arg != -1) ? second_arg : dfu->data.scsi.lun;
		dfu->data.scsi.part = third_arg;
	}

	if (!strcmp(entity_type, "raw")) {
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.scsi.lba_start = second_arg;
		if (third_arg < 0) {
			pr_err("raw requires two arguments\n");
			return -EINVAL;
		}
		dfu->data.scsi.lba_size = third_arg;
		dfu->data.scsi.lba_blk_size = blk_dev->blksz;

		/*
		 * In case the size is zero (i.e. scsi raw 0x10 0),
		 * assume the user intends to use whole device.
		 */
		if (third_arg == 0)
			dfu->data.scsi.lba_size = blk_dev->lba;

	} else if (!strcmp(entity_type, "part")) {
		struct disk_partition partinfo;
		int scsipart = second_arg;

		if (third_arg >= 0) {
			pr_err("part only accepts one argument\n");
			return -EINVAL;
		}

		if (part_get_info(blk_dev, scsipart, &partinfo) != 0) {
			pr_err("Couldn't find part #%d on scsi device #%d\n", scsipart,
			       dfu->data.scsi.lun);
			return -ENODEV;
		}

		dfu->layout = DFU_RAW_ADDR;
		dfu->data.scsi.lba_start = partinfo.start;
		dfu->data.scsi.lba_size = partinfo.size;
		dfu->data.scsi.lba_blk_size = partinfo.blksz;
	} else if (!strcmp(entity_type, "fat")) {
		dfu->layout = DFU_FS_FAT;
	} else if (!strcmp(entity_type, "ext4")) {
		dfu->layout = DFU_FS_EXT4;
	} else if (!strcmp(entity_type, "skip")) {
		dfu->layout = DFU_SKIP;
	} else if (!strcmp(entity_type, "script")) {
		dfu->layout = DFU_SCRIPT;
	} else {
		pr_err("Memory layout (%s) not supported!\n", entity_type);
		return -ENODEV;
	}

	dfu->dev_type = DFU_DEV_SCSI;
	dfu->get_medium_size = dfu_get_medium_size_scsi;
	dfu->read_medium = dfu_read_medium_scsi;
	dfu->write_medium = dfu_write_medium_scsi;
	dfu->flush_medium = dfu_flush_medium_scsi;
	dfu->inited = 0;
	dfu->free_entity = dfu_free_entity_scsi;

	/* Check if file buffer is ready */
	if (!dfu_file_buf) {
		dfu_file_buf = memalign(CONFIG_SYS_CACHELINE_SIZE, CONFIG_SYS_DFU_MAX_FILE_SIZE);
		if (!dfu_file_buf) {
			pr_err("Could not memalign 0x%x bytes\n", CONFIG_SYS_DFU_MAX_FILE_SIZE);
			return -ENOMEM;
		}
	}

	return 0;
}
