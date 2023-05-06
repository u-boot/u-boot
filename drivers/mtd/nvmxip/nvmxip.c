// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include "nvmxip.h"

/**
 * nvmxip_mmio_rawread() - read from the XIP flash
 * @address:	address of the data
 * @value:	pointer to where storing the value read
 *
 * Read raw data from the XIP flash.
 *
 * Return:
 *
 * Always return 0.
 */
static int nvmxip_mmio_rawread(const phys_addr_t address, u64 *value)
{
	*value = readq(address);
	return 0;
}

/**
 * nvmxip_blk_read() - block device read operation
 * @dev:	the block device
 * @blknr:	first block number to read from
 * @blkcnt:	number of blocks to read
 * @buffer:	destination buffer
 *
 * Read data from the block storage device.
 *
 * Return:
 *
 * number of blocks read on success. Otherwise, failure
 */
static ulong nvmxip_blk_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt, void *buffer)
{
	struct nvmxip_plat *plat = dev_get_plat(dev->parent);
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	/* number of the u64 words to read */
	u32 qwords = (blkcnt * desc->blksz) / sizeof(u64);
	/* physical address of the first block to read */
	phys_addr_t blkaddr = plat->phys_base + blknr * desc->blksz;
	u64 *virt_blkaddr;
	u64 *pdst = buffer;
	uint qdata_idx;

	if (!pdst)
		return -EINVAL;

	log_debug("[%s]: reading from blknr: %lu , blkcnt: %lu\n", dev->name, blknr, blkcnt);

	virt_blkaddr = map_sysmem(blkaddr, 0);

	/* assumption: the data is virtually contiguous */

	for (qdata_idx = 0 ; qdata_idx < qwords ; qdata_idx++)
		nvmxip_mmio_rawread((phys_addr_t)(virt_blkaddr + qdata_idx), pdst++);

	log_debug("[%s]:     src[0]: 0x%llx , dst[0]: 0x%llx , src[-1]: 0x%llx , dst[-1]: 0x%llx\n",
		  dev->name,
		  *virt_blkaddr,
		  *(u64 *)buffer,
		  *(u64 *)((u8 *)virt_blkaddr + desc->blksz * blkcnt - sizeof(u64)),
		  *(u64 *)((u8 *)buffer + desc->blksz * blkcnt - sizeof(u64)));

	unmap_sysmem(virt_blkaddr);

	return blkcnt;
}

/**
 * nvmxip_blk_probe() - block storage device probe
 * @dev:	the block storage device
 *
 * Initialize the block storage descriptor.
 *
 * Return:
 *
 * Always return 0.
 */
static int nvmxip_blk_probe(struct udevice *dev)
{
	struct nvmxip_plat *plat = dev_get_plat(dev->parent);
	struct blk_desc *desc = dev_get_uclass_plat(dev);

	desc->lba = plat->lba;
	desc->log2blksz = plat->lba_shift;
	desc->blksz = BIT(plat->lba_shift);
	desc->bdev = dev;

	log_debug("[%s]: block storage layout\n    lbas: %lu , log2blksz: %d, blksz: %lu\n",
		  dev->name, desc->lba, desc->log2blksz, desc->blksz);

	return 0;
}

static const struct blk_ops nvmxip_blk_ops = {
	.read	= nvmxip_blk_read,
};

U_BOOT_DRIVER(nvmxip_blk) = {
	.name	= NVMXIP_BLKDRV_NAME,
	.id	= UCLASS_BLK,
	.probe	= nvmxip_blk_probe,
	.ops	= &nvmxip_blk_ops,
};
