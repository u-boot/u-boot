// SPDX-License-Identifier: GPL-2.0+
/*
 * MTD backend for imagemap (UCLASS_IMAGEMAP driver)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#include <dm.h>
#include <imagemap.h>
#include <mtd.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/err.h>
#include <log.h>

/**
 * struct imagemap_mtd_plat - Platform data set before probe
 *
 * @mtd:	MTD device (resolved by create function, ref held)
 */
struct imagemap_mtd_plat {
	struct mtd_info *mtd;
};

/**
 * struct imagemap_mtd_priv - Runtime private data
 *
 * @mtd:	MTD device
 */
struct imagemap_mtd_priv {
	struct mtd_info *mtd;
};

static int imagemap_mtd_read(struct udevice *dev, loff_t src,
			     ulong size, void *dst)
{
	struct imagemap_mtd_priv *priv = dev_get_priv(dev);
	struct mtd_info *mtd = priv->mtd;
	size_t retlen;
	int ret;

	ret = mtd_read_skip_bad(mtd, src, size, &retlen, dst);
	if (ret)
		return ret;

	return (retlen == size) ? 0 : -EIO;
}

static int imagemap_mtd_probe(struct udevice *dev)
{
	struct imagemap_mtd_plat *plat = dev_get_plat(dev);
	struct imagemap_mtd_priv *priv = dev_get_priv(dev);

	priv->mtd = plat->mtd;

	return 0;
}

static int imagemap_mtd_remove(struct udevice *dev)
{
	struct imagemap_mtd_priv *priv = dev_get_priv(dev);

	if (priv->mtd)
		put_mtd_device(priv->mtd);

	return 0;
}

static int imagemap_create_mtd(const char *name, struct udevice **devp)
{
	struct imagemap_mtd_plat *plat;
	struct udevice *parent;
	struct udevice *dev;
	struct mtd_info *mtd;
	int ret;

	mtd_probe_devices();

	mtd = get_mtd_device_nm(name);
	if (IS_ERR_OR_NULL(mtd)) {
		log_debug("imagemap_mtd: MTD device \"%s\" not found\n", name);
		return -ENODEV;
	}

	/* Use the MTD's DM device as parent if available */
	parent = mtd->dev ? mtd->dev : dm_root();

	ret = device_bind_driver(parent, "imagemap_mtd",
				 name, &dev);
	if (ret) {
		put_mtd_device(mtd);
		return ret;
	}

	plat = dev_get_plat(dev);
	plat->mtd = mtd;

	ret = device_probe(dev);
	if (ret) {
		device_unbind(dev);
		put_mtd_device(mtd);
		return ret;
	}

	*devp = dev;

	return 0;
}

static const struct imagemap_ops imagemap_mtd_ops = {
	.read	= imagemap_mtd_read,
};

U_BOOT_DRIVER(imagemap_mtd) = {
	.name		= "imagemap_mtd",
	.id		= UCLASS_IMAGEMAP,
	.ops		= &imagemap_mtd_ops,
	.probe		= imagemap_mtd_probe,
	.remove		= imagemap_mtd_remove,
	.plat_auto	= sizeof(struct imagemap_mtd_plat),
	.priv_auto	= sizeof(struct imagemap_mtd_priv),
};

static int imagemap_mtd_dev_create(struct udevice *dev, const char *name,
				   int part, struct udevice **devp)
{
	return imagemap_create_mtd(name, devp);
}

IMAGEMAP_BACKEND(imagemap_mtd) = {
	.uclass	= UCLASS_MTD,
	.create	= imagemap_mtd_dev_create,
};

IMAGEMAP_BACKEND(imagemap_mtd_sf) = {
	.uclass	= UCLASS_SPI_FLASH,
	.create	= imagemap_mtd_dev_create,
};
