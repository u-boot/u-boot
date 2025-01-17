// SPDX-License-Identifier: GPL-2.0+
/*
 * blkmap helper function
 *
 * Copyright (c) 2023, Linaro Limited
 */

#include <blk.h>
#include <blkmap.h>
#include <fdt_support.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <linux/kernel.h>

int blkmap_create_ramdisk(const char *label, ulong image_addr, ulong image_size,
			  struct udevice **devp)
{
	int ret;
	lbaint_t blknum;
	struct blkmap *bm;
	struct blk_desc *desc;
	struct udevice *bm_dev;

	ret = blkmap_create(label, &bm_dev, BLKMAP_MEM);
	if (ret) {
		log_err("failed to create blkmap\n");
		return ret;
	}

	bm = dev_get_plat(bm_dev);
	desc = dev_get_uclass_plat(bm->blk);
	blknum = image_size >> desc->log2blksz;
	ret = blkmap_map_pmem(bm_dev, 0, blknum, image_addr);
	if (ret) {
		log_err("Unable to map %#llx at block %d : %d\n",
			(unsigned long long)image_addr, 0, ret);
		goto err;
	}
	log_info("Block %d+0x" LBAF " mapped to %#llx\n", 0, blknum,
		 (unsigned long long)image_addr);

	ret = device_probe(bm->blk);
	if (ret)
		goto err;

	if (devp)
		*devp = bm_dev;

	return 0;

err:
	blkmap_destroy(bm_dev);

	return ret;
}

static int blkmap_add_pmem_node(void *fdt, struct blkmap *bm)
{
	int ret;
	u32 size;
	ulong addr;
	struct blkmap_mem *bmm;
	struct blkmap_slice *bms;
	struct blk_desc *bd = dev_get_uclass_plat(bm->blk);

	list_for_each_entry(bms, &bm->slices, node) {
		bmm = container_of(bms, struct blkmap_mem, slice);

		addr = (ulong)bmm->addr;
		size = (u32)bms->blkcnt << bd->log2blksz;

		ret = fdt_fixup_pmem_region(fdt, addr, size);
		if (ret)
			return ret;
	}

	return 0;
}

int blkmap_fdt_pmem_setup(void *fdt)
{
	int ret;
	struct udevice *dev;
	struct uclass *uc;
	struct blkmap *bm;

	uclass_id_foreach_dev(UCLASS_BLKMAP, dev, uc) {
		bm = dev_get_plat(dev);
		if (bm->type == BLKMAP_MEM) {
			ret = blkmap_add_pmem_node(fdt, bm);
			if (ret)
				return ret;
		}
	}

	return 0;
}
