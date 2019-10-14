// SPDX-License-Identifier: GPL-2.0+
/*
 * dfu_mtd.c -- DFU for MTD device.
 *
 * Copyright (C) 2019,STMicroelectronics - All Rights Reserved
 *
 * Based on dfu_nand.c
 */

#include <common.h>
#include <dfu.h>
#include <mtd.h>

static bool mtd_is_aligned_with_block_size(struct mtd_info *mtd, u64 size)
{
	return !do_div(size, mtd->erasesize);
}

static int mtd_block_op(enum dfu_op op, struct dfu_entity *dfu,
			u64 offset, void *buf, long *len)
{
	u64 off, lim, remaining;
	struct mtd_info *mtd = dfu->data.mtd.info;
	struct mtd_oob_ops io_op = {};
	int ret = 0;
	bool has_pages = mtd->type == MTD_NANDFLASH ||
			 mtd->type == MTD_MLCNANDFLASH;

	/* if buf == NULL return total size of the area */
	if (!buf) {
		*len = dfu->data.mtd.size;
		return 0;
	}

	off = dfu->data.mtd.start + offset + dfu->bad_skip;
	lim = dfu->data.mtd.start + dfu->data.mtd.size;

	if (off >= lim) {
		printf("Limit reached 0x%llx\n", lim);
		*len = 0;
		return op == DFU_OP_READ ? 0 : -EIO;
	}
	/* limit request with the available size */
	if (off + *len >= lim)
		*len = lim - off;

	if (!mtd_is_aligned_with_block_size(mtd, off)) {
		printf("Offset not aligned with a block (0x%x)\n",
		       mtd->erasesize);
		return 0;
	}

	/* first erase */
	if (op == DFU_OP_WRITE) {
		struct erase_info erase_op = {};

		remaining = round_up(*len, mtd->erasesize);
		erase_op.mtd = mtd;
		erase_op.addr = off;
		erase_op.len = mtd->erasesize;
		erase_op.scrub = 0;

		while (remaining) {
			if (erase_op.addr + remaining > lim) {
				printf("Limit reached 0x%llx while erasing at offset 0x%llx\n",
				       lim, off);
				return -EIO;
			}

			ret = mtd_erase(mtd, &erase_op);

			if (ret) {
				/* Abort if its not a bad block error */
				if (ret != -EIO) {
					printf("Failure while erasing at offset 0x%llx\n",
					       erase_op.fail_addr);
					return 0;
				}
				printf("Skipping bad block at 0x%08llx\n",
				       erase_op.addr);
			} else {
				remaining -= mtd->erasesize;
			}

			/* Continue erase behind bad block */
			erase_op.addr += mtd->erasesize;
		}
	}

	io_op.mode = MTD_OPS_AUTO_OOB;
	io_op.len = *len;
	if (has_pages && io_op.len > mtd->writesize)
		io_op.len = mtd->writesize;
	io_op.ooblen = 0;
	io_op.datbuf = buf;
	io_op.oobbuf = NULL;

	/* Loop over to do the actual read/write */
	remaining = *len;
	while (remaining) {
		if (off + remaining > lim) {
			printf("Limit reached 0x%llx while %s at offset 0x%llx\n",
			       lim, op == DFU_OP_READ ? "reading" : "writing",
			       off);
			if (op == DFU_OP_READ) {
				*len -= remaining;
				return 0;
			} else {
				return -EIO;
			}
		}

		/* Skip the block if it is bad */
		if (mtd_is_aligned_with_block_size(mtd, off) &&
		    mtd_block_isbad(mtd, off)) {
			off += mtd->erasesize;
			dfu->bad_skip += mtd->erasesize;
			continue;
		}

		if (op == DFU_OP_READ)
			ret = mtd_read_oob(mtd, off, &io_op);
		else
			ret = mtd_write_oob(mtd, off, &io_op);

		if (ret) {
			printf("Failure while %s at offset 0x%llx\n",
			       op == DFU_OP_READ ? "reading" : "writing", off);
			return -EIO;
		}

		off += io_op.retlen;
		remaining -= io_op.retlen;
		io_op.datbuf += io_op.retlen;
		io_op.len = remaining;
		if (has_pages && io_op.len > mtd->writesize)
			io_op.len = mtd->writesize;
	}

	return ret;
}

static int dfu_get_medium_size_mtd(struct dfu_entity *dfu, u64 *size)
{
	*size = dfu->data.mtd.info->size;

	return 0;
}

static int dfu_read_medium_mtd(struct dfu_entity *dfu, u64 offset, void *buf,
			       long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mtd_block_op(DFU_OP_READ, dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

static int dfu_write_medium_mtd(struct dfu_entity *dfu,
				u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mtd_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

static int dfu_flush_medium_mtd(struct dfu_entity *dfu)
{
	return 0;
}

static unsigned int dfu_polltimeout_mtd(struct dfu_entity *dfu)
{
	return DFU_DEFAULT_POLL_TIMEOUT;
}

int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr, char *s)
{
	char *st;
	struct mtd_info *mtd;
	bool has_pages;

	mtd = get_mtd_device_nm(devstr);
	if (IS_ERR_OR_NULL(mtd))
		return -ENODEV;
	put_mtd_device(mtd);

	dfu->dev_type = DFU_DEV_MTD;
	dfu->data.mtd.info = mtd;

	has_pages = mtd->type == MTD_NANDFLASH || mtd->type == MTD_MLCNANDFLASH;
	dfu->max_buf_size = has_pages ? mtd->erasesize : 0;

	st = strsep(&s, " ");
	if (!strcmp(st, "raw")) {
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.mtd.start = simple_strtoul(s, &s, 16);
		s++;
		dfu->data.mtd.size = simple_strtoul(s, &s, 16);
	} else {
		printf("%s: (%s) not supported!\n", __func__, st);
		return -1;
	}

	dfu->get_medium_size = dfu_get_medium_size_mtd;
	dfu->read_medium = dfu_read_medium_mtd;
	dfu->write_medium = dfu_write_medium_mtd;
	dfu->flush_medium = dfu_flush_medium_mtd;
	dfu->poll_timeout = dfu_polltimeout_mtd;

	/* initial state */
	dfu->inited = 0;

	return 0;
}
