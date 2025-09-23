// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 *
 */

#include <errno.h>
#include <blk.h>
#include <linux/types.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <fw_loader.h>

#ifdef CONFIG_CMD_UBIFS
#include <ubi_uboot.h>
#endif

#include "internal.h"

#ifdef CONFIG_CMD_UBIFS
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

int generic_fw_loader_of_to_plat(struct udevice *dev)
{
	u32 phandlepart[2];

	ofnode fw_loader_node = dev_ofnode(dev);

	if (ofnode_valid(fw_loader_node)) {
		struct device_plat *plat;

		plat = dev_get_plat(dev);
		if (!ofnode_read_u32_array(fw_loader_node,
					   "phandlepart",
					   phandlepart, 2)) {
			plat->phandlepart.phandle = phandlepart[0];
			plat->phandlepart.partition = phandlepart[1];
		}

		plat->mtdpart = (char *)ofnode_read_string(fw_loader_node,
							   "mtdpart");

		plat->ubivol = (char *)ofnode_read_string(fw_loader_node,
							  "ubivol");
	}

	return 0;
}

int generic_fw_loader_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(BLK)
	int ret;
	struct device_plat *plat = dev_get_plat(dev);

	if (plat->phandlepart.phandle) {
		ofnode node = ofnode_get_by_phandle(plat->phandlepart.phandle);
		struct udevice *parent_dev = NULL;

		ret = device_get_global_by_ofnode(node, &parent_dev);
		if (!ret) {
			struct udevice *blk_dev;

			ret = blk_get_from_parent(parent_dev, &blk_dev);
			if (ret) {
				debug("fw_loader: No block device: %d\n",
				      ret);

				return ret;
			}
		}
	}
#endif

	return 0;
}

static int fw_loaders[] = {
#if CONFIG_IS_ENABLED(FS_LOADER)
	UCLASS_FS_FIRMWARE_LOADER,
#endif
#if CONFIG_IS_ENABLED(FIP_LOADER)
	UCLASS_FIP_FIRMWARE_LOADER,
#endif
};

/**
 * get_fw_loader_from_node - Get FW loader dev from @node.
 *
 * @node: ofnode where "firmware-loader" phandle is stored.
 * @dev: pointer where to store the FW loader dev.
 *
 * Loop over all the supported FW loader and find a matching
 * one.
 *
 * Return: Negative value if fail, 0 for successful.
 */
int get_fw_loader_from_node(ofnode node, struct udevice **dev)
{
	int i, ret;

	node = ofnode_parse_phandle(node, "firmware-loader", 0);
	if (!ofnode_valid(node))
		return -ENODEV;

	/*
	 * Loop over all the available FW loaders and stop when
	 * found one.
	 */
	for (i = 0; i < ARRAY_SIZE(fw_loaders); i++) {
		ret = uclass_get_device_by_ofnode(fw_loaders[i],
						  node, dev);
		if (!ret)
			return 0;
	}

	return -ENODEV;
}

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
	if (!name || name[0] == '\0')
		return -EINVAL;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -ENOMEM;

	firmwarep->name = name;
	firmwarep->offset = offset;
	firmwarep->data = dbuf;
	firmwarep->size = size;

	return 0;
}

int request_firmware_into_buf(struct udevice *dev,
			      const char *name,
			      void *buf, size_t size, u32 offset)
{
	struct device_plat *plat;
	int ret;

	if (!dev)
		return -EINVAL;

	ret = _request_firmware_prepare(dev, name, buf, size, offset);
	if (ret < 0) /* error */
		return ret;

	plat = dev_get_plat(dev);

	if (!plat->get_firmware)
		return -EOPNOTSUPP;

	return plat->get_firmware(dev);
}
