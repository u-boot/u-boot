// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 EPAM Systems Inc.
 */
#include <blk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>

#define DRV_NAME	"pvblock"
#define DRV_NAME_BLK	"pvblock_blk"

struct blkfront_dev {
	char dummy;
};

static int init_blkfront(unsigned int devid, struct blkfront_dev *dev)
{
	return 0;
}

static void shutdown_blkfront(struct blkfront_dev *dev)
{
}

ulong pvblock_blk_read(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
{
	return 0;
}

ulong pvblock_blk_write(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
{
	return 0;
}

static int pvblock_blk_bind(struct udevice *udev)
{
	return 0;
}

static int pvblock_blk_probe(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);
	int ret;

	ret = init_blkfront(0, blk_dev);
	if (ret < 0)
		return ret;
	return 0;
}

static int pvblock_blk_remove(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);

	shutdown_blkfront(blk_dev);
	return 0;
}

static const struct blk_ops pvblock_blk_ops = {
	.read	= pvblock_blk_read,
	.write	= pvblock_blk_write,
};

U_BOOT_DRIVER(pvblock_blk) = {
	.name			= DRV_NAME_BLK,
	.id			= UCLASS_BLK,
	.ops			= &pvblock_blk_ops,
	.bind			= pvblock_blk_bind,
	.probe			= pvblock_blk_probe,
	.remove			= pvblock_blk_remove,
	.priv_auto_alloc_size	= sizeof(struct blkfront_dev),
	.flags			= DM_FLAG_OS_PREPARE,
};

/*******************************************************************************
 * Para-virtual block device class
 *******************************************************************************/

void pvblock_init(void)
{
	struct driver_info info;
	struct udevice *udev;
	struct uclass *uc;
	int ret;

	/*
	 * At this point Xen drivers have already initialized,
	 * so we can instantiate the class driver and enumerate
	 * virtual block devices.
	 */
	info.name = DRV_NAME;
	ret = device_bind_by_name(gd->dm_root, false, &info, &udev);
	if (ret < 0)
		printf("Failed to bind " DRV_NAME ", ret: %d\n", ret);

	/* Bootstrap virtual block devices class driver */
	ret = uclass_get(UCLASS_PVBLOCK, &uc);
	if (ret)
		return;
	uclass_foreach_dev_probe(UCLASS_PVBLOCK, udev);
}

static int pvblock_probe(struct udevice *udev)
{
	return 0;
}

U_BOOT_DRIVER(pvblock_drv) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
	.probe		= pvblock_probe,
};

UCLASS_DRIVER(pvblock) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
};
