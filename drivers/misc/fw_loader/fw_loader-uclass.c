// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 *
 */

#include <errno.h>
#include <blk.h>
#include <linux/types.h>
#include <dm.h>
#include <dm/device.h>
#include <fw_loader.h>

#if CONFIG_IS_ENABLED(CMD_UBIFS)
#include <ubi_uboot.h>
#endif

#include "internal.h"

#if CONFIG_IS_ENABLED(CMD_UBIFS)
int generic_fw_loader_ubi_select(char *mtdpart)
{
	int ret;

	ret = ubi_part(mtdpart, NULL);
	if (ret)
		debug("Cannot find mtd partition %s\n", mtdpart);

	return ret;
}
#else
int generic_fw_loader_ubi_select(char *mtdpart)
{
	debug("Error: Cannot select ubi partition: no UBIFS support\n");
	return -ENOSYS;
}
#endif

#if CONFIG_IS_ENABLED(BLK)
static int fw_loader_get_blk_dev_from_node(ofnode node)
{
	struct udevice *parent_dev = NULL;
	struct udevice *blk_dev;
	int ret;

	ret = device_get_global_by_ofnode(node, &parent_dev);
	if (ret)
		return ret;

	ret = blk_get_from_parent(parent_dev, &blk_dev);
	if (ret)
		debug("fw_loader: No block device: %d\n", ret);

	return ret;
}
#else
static int fw_loader_get_blk_dev_from_node(ofnode node)
{
	debug("fw_loader: No block device support\n");
	return -EINVAL;
}
#endif

static int fw_loader_pre_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM)
	struct device_plat *plat = dev_get_uclass_plat(dev);
	ofnode fw_loader_node;

	if (!IS_ENABLED(CONFIG_DM))
		return 0;

	fw_loader_node = dev_ofnode(dev);
	if (ofnode_valid(fw_loader_node)) {
		u32 phandlepart[2];

		if (!ofnode_read_u32_array(fw_loader_node, "phandlepart",
					   phandlepart, 2)) {
			plat->phandlepart.phandle = phandlepart[0];
			plat->phandlepart.partition = phandlepart[1];
		}

		plat->mtdpart = (char *)ofnode_read_string(fw_loader_node,
							   "mtdpart");

		plat->ubivol = (char *)ofnode_read_string(fw_loader_node,
							  "ubivol");
	}

	if (plat->phandlepart.phandle) {
		ofnode node = ofnode_get_by_phandle(plat->phandlepart.phandle);
		int ret;

		ret = fw_loader_get_blk_dev_from_node(node);
		if (ret)
			return ret;
	}
#endif

	return 0;
}

UCLASS_DRIVER(fw_loader) = {
	.id		= UCLASS_FIRMWARE_LOADER,
	.name		= "fw_loader",
	.pre_probe	= fw_loader_pre_probe,
	.per_device_plat_auto = sizeof(struct device_plat),
	.per_device_auto = sizeof(struct firmware),
};

/**
 * _request_firmware_prepare - Prepare firmware struct.
 *
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 * @dbuf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 *
 * Return: Negative value if fail, 0 for successful.
 */
static int _request_firmware_prepare(struct udevice *dev,
				     const char *name, void *dbuf,
				     size_t size, u32 offset)
{
	struct firmware *upriv = dev_get_uclass_priv(dev);

	if (!name || name[0] == '\0')
		return -EINVAL;

	upriv->name = name;
	upriv->offset = offset;
	upriv->data = dbuf;
	upriv->size = size;

	return 0;
}

int request_firmware_into_buf(struct udevice *dev,
			      const char *name,
			      void *buf, size_t size, u32 offset)
{
	struct fw_loader_ops *ops;
	int ret;

	if (!dev)
		return -EINVAL;

	ret = _request_firmware_prepare(dev, name, buf, size, offset);
	if (ret < 0) /* error */
		return ret;

	ops = fw_loader_get_ops(dev);

	if (!ops->get_firmware)
		return -EOPNOTSUPP;

	return ops->get_firmware(dev);
}
