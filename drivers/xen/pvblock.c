// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 EPAM Systems Inc.
 */
#include <blk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <part.h>

#include <xen/xenbus.h>

#define DRV_NAME	"pvblock"
#define DRV_NAME_BLK	"pvblock_blk"

struct blkfront_dev {
	char dummy;
};

struct blkfront_platdata {
	unsigned int devid;
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
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	int devnum;

	desc->if_type = IF_TYPE_PVBLOCK;
	/*
	 * Initialize the devnum to -ENODEV. This is to make sure that
	 * blk_next_free_devnum() works as expected, since the default
	 * value 0 is a valid devnum.
	 */
	desc->devnum = -ENODEV;
	devnum = blk_next_free_devnum(IF_TYPE_PVBLOCK);
	if (devnum < 0)
		return devnum;
	desc->devnum = devnum;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = udev;

	strncpy(desc->vendor, "Xen", sizeof(desc->vendor));
	strncpy(desc->revision, "1", sizeof(desc->revision));
	strncpy(desc->product, "Virtual disk", sizeof(desc->product));

	return 0;
}

static int pvblock_blk_probe(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);
	struct blkfront_platdata *platdata = dev_get_platdata(udev);
	int ret, devid;

	devid = platdata->devid;
	free(platdata);

	ret = init_blkfront(devid, blk_dev);
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

typedef int (*enum_vbd_callback)(struct udevice *parent, unsigned int devid);

static int on_new_vbd(struct udevice *parent, unsigned int devid)
{
	struct driver_info info;
	struct udevice *udev;
	struct blkfront_platdata *platdata;
	int ret;

	debug("New " DRV_NAME_BLK ", device ID %d\n", devid);

	platdata = malloc(sizeof(struct blkfront_platdata));
	if (!platdata) {
		printf("Failed to allocate platform data\n");
		return -ENOMEM;
	}

	platdata->devid = devid;

	info.name = DRV_NAME_BLK;
	info.platdata = platdata;

	ret = device_bind_by_name(parent, false, &info, &udev);
	if (ret < 0) {
		printf("Failed to bind " DRV_NAME_BLK " to device with ID %d, ret: %d\n",
		       devid, ret);
		free(platdata);
	}
	return ret;
}

static int xenbus_enumerate_vbd(struct udevice *udev, enum_vbd_callback clb)
{
	char **dirs, *msg;
	int i, ret;

	msg = xenbus_ls(XBT_NIL, "device/vbd", &dirs);
	if (msg) {
		printf("Failed to read device/vbd directory: %s\n", msg);
		free(msg);
		return -ENODEV;
	}

	for (i = 0; dirs[i]; i++) {
		int devid;

		sscanf(dirs[i], "%d", &devid);
		ret = clb(udev, devid);
		if (ret < 0)
			goto fail;

		free(dirs[i]);
	}
	ret = 0;

fail:
	for (; dirs[i]; i++)
		free(dirs[i]);
	free(dirs);
	return ret;
}

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
	struct uclass *uc;
	int ret;

	if (xenbus_enumerate_vbd(udev, on_new_vbd) < 0)
		return -ENODEV;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_probe(UCLASS_BLK, udev) {
		if (_ret)
			return _ret;
	};
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
