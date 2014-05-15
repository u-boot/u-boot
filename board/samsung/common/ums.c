/*
 * Copyright (C) 2013 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb_mass_storage.h>
#include <part.h>

static int ums_read_sector(struct ums *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf)
{
	block_dev_desc_t *block_dev = &ums_dev->mmc->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_read(dev_num, blkstart, blkcnt, buf);
}

static int ums_write_sector(struct ums *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf)
{
	block_dev_desc_t *block_dev = &ums_dev->mmc->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_write(dev_num, blkstart, blkcnt, buf);
}

static struct ums ums_dev = {
	.read_sector = ums_read_sector,
	.write_sector = ums_write_sector,
	.name = "UMS disk",
};

static struct ums *ums_disk_init(struct mmc *mmc)
{
	uint64_t mmc_end_sector = mmc->capacity / SECTOR_SIZE;
	uint64_t ums_end_sector = UMS_NUM_SECTORS + UMS_START_SECTOR;

	if (!mmc_end_sector) {
		error("MMC capacity is not valid");
		return NULL;
	}

	ums_dev.mmc = mmc;

	if (ums_end_sector <= mmc_end_sector) {
		ums_dev.start_sector = UMS_START_SECTOR;
		if (UMS_NUM_SECTORS)
			ums_dev.num_sectors = UMS_NUM_SECTORS;
		else
			ums_dev.num_sectors = mmc_end_sector - UMS_START_SECTOR;
	} else {
		ums_dev.num_sectors = mmc_end_sector;
		puts("UMS: defined bad disk parameters. Using default.\n");
	}

	printf("UMS: disk start sector: %#x, count: %#x\n",
	       ums_dev.start_sector, ums_dev.num_sectors);

	return &ums_dev;
}

struct ums *ums_init(unsigned int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (!mmc || mmc_init(mmc))
		return NULL;
	return ums_disk_init(mmc);
}
