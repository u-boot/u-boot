// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 SaluteDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <blk.h>
#include <part.h>
#include <ubi_uboot.h>
#include <dm/device.h>
#include <dm/device-internal.h>

int ubi_bind(struct udevice *dev)
{
	struct blk_desc *bdesc;
	struct udevice *bdev;
	int ret;

	ret = blk_create_devicef(dev, "ubi_blk", "blk", UCLASS_MTD,
				 -1, 512, 0, &bdev);
	if (ret) {
		pr_err("Cannot create block device");
		return ret;
	}

	bdesc = dev_get_uclass_plat(bdev);

	bdesc->bdev = bdev;
	bdesc->part_type = PART_TYPE_UBI;

	return 0;
}

static struct ubi_device *get_ubi_device(void)
{
	return ubi_devices[0];
}

static char *get_volume_name(int vol_id)
{
	struct ubi_device *ubi = get_ubi_device();
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		struct ubi_volume *volume = ubi->volumes[i];

		if (!volume)
			continue;

		if (volume->vol_id >= UBI_INTERNAL_VOL_START)
			continue;

		if (volume->vol_id == vol_id)
			return volume->name;
	}

	return NULL;
}

static ulong ubi_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		       void *dst)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	char *volume_name = get_volume_name(block_dev->hwpart);
	unsigned int size = blkcnt * block_dev->blksz;
	loff_t offset = start * block_dev->blksz;
	int ret;

	if (!volume_name) {
		pr_err("%s: failed to find volume name for blk=" LBAF "\n", __func__, start);
		return -EINVAL;
	}

	ret = ubi_volume_read(volume_name, dst, offset, size);
	if (ret) {
		pr_err("%s: failed to read from %s UBI volume\n", __func__, volume_name);
		return ret;
	}

	return blkcnt;
}

static ulong ubi_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
			const void *src)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	char *volume_name = get_volume_name(block_dev->hwpart);
	unsigned int size = blkcnt * block_dev->blksz;
	loff_t offset = start * block_dev->blksz;
	int ret;

	if (!volume_name) {
		pr_err("%s: failed to find volume for blk=" LBAF "\n", __func__, start);
		return -EINVAL;
	}

	ret = ubi_volume_write(volume_name, (void *)src, offset, size);
	if (ret) {
		pr_err("%s: failed to write from %s UBI volume\n", __func__, volume_name);
		return ret;
	}

	return blkcnt;
}

static int ubi_blk_probe(struct udevice *dev)
{
	int ret;

	ret = device_probe(dev);
	if (ret) {
		pr_err("Probing %s failed (err=%d)\n", dev->name, ret);
		return ret;
	}

	return 0;
}

static const struct blk_ops ubi_blk_ops = {
	.read = ubi_bread,
	.write = ubi_bwrite,
};

U_BOOT_DRIVER(ubi_blk) = {
	.name = "ubi_blk",
	.id = UCLASS_BLK,
	.ops = &ubi_blk_ops,
	.probe = ubi_blk_probe,
};
