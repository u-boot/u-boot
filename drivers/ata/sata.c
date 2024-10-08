// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 */

#define LOG_CATEGORY UCLASS_AHCI

#include <ahci.h>
#include <blk.h>
#include <dm.h>
#include <log.h>
#include <part.h>
#include <sata.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

int sata_reset(struct udevice *dev)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->reset)
		return -ENOSYS;

	return ops->reset(dev);
}

int sata_dm_port_status(struct udevice *dev, int port)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->port_status)
		return -ENOSYS;

	return ops->port_status(dev, port);
}

int sata_scan(struct udevice *dev)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->scan)
		return -ENOSYS;

	return ops->scan(dev);
}

int sata_rescan(bool verbose)
{
	struct uclass *uc;
	struct udevice *dev; /* SATA controller */
	int ret;

	if (verbose)
		printf("scanning bus for devices...\n");

	ret = uclass_get(UCLASS_AHCI, &uc);
	if (ret)
		return ret;

	/* Remove all children of SATA devices (blk and bootdev) */
	uclass_foreach_dev(dev, uc) {
		log_debug("unbind %s\n", dev->name);
		ret = device_chld_remove(dev, NULL, DM_REMOVE_NORMAL);
		if (!ret)
			ret = device_chld_unbind(dev, NULL);
		if (ret && verbose) {
			log_err("Unbinding from %s failed (%dE)\n",
				dev->name, ret);
		}
	}

	if (verbose)
		printf("Rescanning SATA bus for devices...\n");

	uclass_foreach_dev_probe(UCLASS_AHCI, dev) {
		ret = sata_scan(dev);
		if (ret && verbose)
			log_err("Scanning %s failed (%dE)\n", dev->name, ret);
	}

	return 0;
}

static unsigned long sata_bread(struct udevice *dev, lbaint_t start,
				lbaint_t blkcnt, void *dst)
{
	return -ENOSYS;
}

static unsigned long sata_bwrite(struct udevice *dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	return -ENOSYS;
}

static const struct blk_ops sata_blk_ops = {
	.read	= sata_bread,
	.write	= sata_bwrite,
};

U_BOOT_DRIVER(sata_blk) = {
	.name		= "sata_blk",
	.id		= UCLASS_BLK,
	.ops		= &sata_blk_ops,
};
