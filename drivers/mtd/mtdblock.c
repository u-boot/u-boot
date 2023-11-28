// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * MTD block - abstraction over MTD subsystem, allowing
 * to read and write in blocks using BLK UCLASS.
 *
 * - Read algorithm:
 *
 *   1. Convert start block number to start address.
 *   2. Read block_dev->blksz bytes using mtd_read() and
 *      add to start address pointer block_dev->blksz bytes,
 *      until the requested number of blocks have been read.
 *
 * - Write algorithm:
 *
 *   1. Convert start block number to start address.
 *   2. Round this address down by mtd->erasesize.
 *
 *   Erase addr      Start addr
 *      |                |
 *      v                v
 *      +----------------+----------------+----------------+
 *      |     blksz      |      blksz     |      blksz     |
 *      +----------------+----------------+----------------+
 *
 *   3. Calculate offset between this two addresses.
 *   4. Read mtd->erasesize bytes using mtd_read() into
 *      temporary buffer from erase address.
 *
 *   Erase addr      Start addr
 *      |                |
 *      v                v
 *      +----------------+----------------+----------------+
 *      |     blksz      |      blksz     |      blksz     |
 *      +----------------+----------------+----------------+
 *      ^
 *      |
 *      |
 *   mtd_read()
 *   from here
 *
 *   5. Copy data from user buffer to temporary buffer with offset,
 *      calculated at step 3.
 *   6. Erase and write mtd->erasesize bytes at erase address
 *      pointer using mtd_erase/mtd_write().
 *   7. Add to erase address pointer mtd->erasesize bytes.
 *   8. goto 1 until the requested number of blocks have
 *      been written.
 *
 * (C) Copyright 2024 SaluteDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <blk.h>
#include <part.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <linux/mtd/mtd.h>

int mtd_bind(struct udevice *dev, struct mtd_info **mtd)
{
	struct blk_desc *bdesc;
	struct udevice *bdev;
	int ret;

	ret = blk_create_devicef(dev, "mtd_blk", "blk", UCLASS_MTD,
				 -1, 512, 0, &bdev);
	if (ret) {
		pr_err("Cannot create block device\n");
		return ret;
	}

	bdesc = dev_get_uclass_plat(bdev);
	dev_set_priv(bdev, mtd);
	bdesc->bdev = bdev;
	bdesc->part_type = PART_TYPE_MTD;

	return 0;
}

static ulong mtd_blk_read(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
			  void *dst)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	struct mtd_info *mtd = blk_desc_to_mtd(block_dev);
	unsigned int sect_size = block_dev->blksz;
	lbaint_t cur = start;
	ulong read_cnt = 0;

	while (read_cnt < blkcnt) {
		int ret;
		loff_t sect_start = cur * sect_size;
		size_t retlen;

		ret = mtd_read(mtd, sect_start, sect_size, &retlen, dst);
		if (ret)
			return ret;

		if (retlen != sect_size) {
			pr_err("mtdblock: failed to read block 0x" LBAF "\n", cur);
			return -EIO;
		}

		cur++;
		dst += sect_size;
		read_cnt++;
	}

	return read_cnt;
}

static int mtd_erase_write(struct mtd_info *mtd, uint64_t start, const void *src)
{
	int ret;
	size_t retlen;
	struct erase_info erase = { 0 };

	erase.mtd = mtd;
	erase.addr = start;
	erase.len = mtd->erasesize;

	ret = mtd_erase(mtd, &erase);
	if (ret)
		return ret;

	ret = mtd_write(mtd, start, mtd->erasesize, &retlen, src);
	if (ret)
		return ret;

	if (retlen != mtd->erasesize) {
		pr_err("mtdblock: failed to read block at 0x%llx\n", start);
		return -EIO;
	}

	return 0;
}

static ulong mtd_blk_write(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
			   const void *src)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	struct mtd_info *mtd = blk_desc_to_mtd(block_dev);
	unsigned int sect_size = block_dev->blksz;
	lbaint_t cur = start, blocks_todo = blkcnt;
	ulong write_cnt = 0;
	u8 *buf;
	int ret = 0;

	buf = malloc(mtd->erasesize);
	if (!buf)
		return -ENOMEM;

	while (blocks_todo > 0) {
		loff_t sect_start = cur * sect_size;
		loff_t erase_start = ALIGN_DOWN(sect_start, mtd->erasesize);
		u32 offset = sect_start - erase_start;
		size_t cur_size = min_t(size_t,  mtd->erasesize - offset,
					blocks_todo * sect_size);
		size_t retlen;
		lbaint_t written;

		ret = mtd_read(mtd, erase_start, mtd->erasesize, &retlen, buf);
		if (ret)
			goto out;

		if (retlen != mtd->erasesize) {
			pr_err("mtdblock: failed to read block 0x" LBAF "\n", cur);
			ret = -EIO;
			goto out;
		}

		memcpy(buf + offset, src, cur_size);

		ret = mtd_erase_write(mtd, erase_start, buf);
		if (ret)
			goto out;

		written = cur_size / sect_size;

		blocks_todo -= written;
		cur += written;
		src += cur_size;
		write_cnt += written;
	}

out:
	free(buf);

	if (ret)
		return ret;

	return write_cnt;
}

static int mtd_blk_probe(struct udevice *dev)
{
	struct blk_desc *bdesc;
	struct mtd_info *mtd;
	int ret;

	ret = device_probe(dev);
	if (ret) {
		pr_err("Probing %s failed (err=%d)\n", dev->name, ret);
		return ret;
	}

	bdesc = dev_get_uclass_plat(dev);
	mtd = blk_desc_to_mtd(bdesc);

	if (mtd_type_is_nand(mtd))
		pr_warn("MTD device '%s' is NAND, please use UBI devices instead\n",
			mtd->name);

	return 0;
}

static const struct blk_ops mtd_blk_ops = {
	.read = mtd_blk_read,
	.write = mtd_blk_write,
};

U_BOOT_DRIVER(mtd_blk) = {
	.name = "mtd_blk",
	.id = UCLASS_BLK,
	.ops = &mtd_blk_ops,
	.probe = mtd_blk_probe,
};
