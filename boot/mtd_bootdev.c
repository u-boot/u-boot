// SPDX-License-Identifier: GPL-2.0+
/*
 * MTD boot device
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>
#include <malloc.h>
#include <mtd.h>

static int mtd_bootdev_get_bootflow(struct udevice *dev,
				    struct bootflow_iter *iter,
				    struct bootflow *bflow)
{
	struct udevice *parent = dev_get_parent(dev);
	struct mtd_info *mtd, *part;
	char dname[80];
	int n = 0;
	int ret;

	ret = bootmeth_check(bflow->method, iter);
	if (ret)
		return log_msg_ret("chk", ret);

	/* Find the top-level MTD device matching our parent */
	mtd_for_each_device(mtd) {
		if (!mtd_is_partition(mtd) && mtd->dev == parent)
			break;
	}
	if (!mtd || mtd->dev != parent)
		return log_msg_ret("mtd", -ESHUTDOWN);

	/* Count partitions so the scanning framework knows the bound */
	list_for_each_entry(part, &mtd->partitions, node)
		n++;
	if (n)
		iter->max_part = n - 1;

	n = 0;

	/* Walk to the iter->part'th sub-partition */
	list_for_each_entry(part, &mtd->partitions, node) {
		if (n == iter->part)
			goto found;
		n++;
	}
	return -ESHUTDOWN;

found:
	/* Device-style name and partition index for bootflow list display */
	snprintf(dname, sizeof(dname), "%s.part_%d", dev->name,
		 iter->part);
	bflow->name = strdup(dname);
	if (!bflow->name)
		return log_msg_ret("nam", -ENOMEM);

	bflow->part = iter->part;
	bflow->fname = strdup(part->name);
	if (!bflow->fname)
		return log_msg_ret("fna", -ENOMEM);

	bflow->state = BOOTFLOWST_MEDIA;

	return bootmeth_read_bootflow(bflow->method, bflow);
}

static int mtd_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST;

	return 0;
}

struct bootdev_ops mtd_bootdev_ops = {
	.get_bootflow	= mtd_bootdev_get_bootflow,
};

static const struct udevice_id mtd_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-mtd" },
	{ }
};

U_BOOT_DRIVER(mtd_bootdev) = {
	.name		= "mtd_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &mtd_bootdev_ops,
	.bind		= mtd_bootdev_bind,
	.of_match	= mtd_bootdev_ids,
};

/**
 * mtd_bootdev_hunt() - probe MTD devices and bind bootdevs
 *
 * Call mtd_probe_devices() to ensure all MTD devices (including SPI NOR
 * flash via CONFIG_SPI_FLASH_MTD) are probed and their partitions parsed.
 *
 * UCLASS_MTD devices already get an mtd_bootdev via mtd_post_bind().
 * This creates mtd_bootdev instances for other MTD devices (e.g. SPI NOR
 * in UCLASS_SPI_FLASH) that have partitions but would otherwise lack one.
 * bootdev_bind() is used directly because sf_bootdev may already occupy
 * the standard bootdev child slot.
 */
static int mtd_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	struct mtd_info *mtd;
	struct udevice *bdev;

	mtd_probe_devices();

	mtd_for_each_device(mtd) {
		/* Only top-level MTD devices, not partitions */
		if (mtd_is_partition(mtd))
			continue;

		/* Must have a DM device */
		if (!mtd->dev)
			continue;

		/* UCLASS_MTD devices already have mtd_bootdev from post_bind */
		if (device_get_uclass_id(mtd->dev) == UCLASS_MTD)
			continue;

		/* Only interested in MTD devices that have partitions */
		if (list_empty(&mtd->partitions))
			continue;

		bootdev_bind(mtd->dev, "mtd_bootdev", "mtdbootdev", &bdev);
	}

	return 0;
}

BOOTDEV_HUNTER(mtd_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_MTD,
	.drv		= DM_DRIVER_REF(mtd_bootdev),
	.hunt		= mtd_bootdev_hunt,
};
