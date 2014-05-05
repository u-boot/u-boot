/*
 * Copyright (C) 2013 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb_mass_storage.h>
#include <mmc.h>
#include <part.h>

static int ums_read_sector(struct ums *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_read(dev_num, blkstart, blkcnt, buf);
}

static int ums_write_sector(struct ums *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_write(dev_num, blkstart, blkcnt, buf);
}

static struct ums ums_dev = {
	.read_sector = ums_read_sector,
	.write_sector = ums_write_sector,
	.name = "UMS disk",
};

struct ums *ums_init(unsigned int dev_num)
{
	struct mmc *mmc = NULL;

	mmc = find_mmc_device(dev_num);
	if (!mmc || mmc_init(mmc))
		return NULL;

	ums_dev.block_dev = &mmc->block_dev;
	ums_dev.start_sector = 0;
	ums_dev.num_sectors = mmc->capacity / SECTOR_SIZE;

	printf("UMS: disk start sector: %#x, count: %#x\n",
	       ums_dev.start_sector, ums_dev.num_sectors);

	return &ums_dev;
}
