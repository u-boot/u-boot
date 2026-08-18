// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 *
 */

#define LOG_CATEGORY UCLASS_FIRMWARE_LOADER

#include <dm.h>
#include <env.h>
#include <errno.h>
#include <blk.h>
#include <fs.h>
#include <fs_loader.h>
#include <log.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <linux/string.h>
#include <mapmem.h>
#include <malloc.h>
#include <spl.h>

#ifdef CONFIG_CMD_UBIFS
#include <ubi_uboot.h>
#endif

#include "internal.h"

#ifdef CONFIG_CMD_UBIFS
static int mount_ubifs(char *mtdpart, char *ubivol)
{
	int ret;

	ret = generic_fw_loader_ubi_select(mtdpart);
	if (ret)
		return ret;

	return cmd_ubifs_mount(ubivol);
}

static int umount_ubifs(void)
{
	return cmd_ubifs_umount();
}
#else
static int mount_ubifs(char *mtdpart, char *ubivol)
{
	debug("Error: Cannot load image: no UBIFS support\n");
	return -ENOSYS;
}
#endif

static int select_fs_dev(struct device_plat *plat)
{
	int ret;

	if (plat->phandlepart.phandle) {
		ofnode node;

		node = ofnode_get_by_phandle(plat->phandlepart.phandle);

		struct udevice *dev;

		ret = device_get_global_by_ofnode(node, &dev);
		if (!ret) {
			struct blk_desc *desc = blk_get_by_device(dev);
			if (desc) {
				ret = fs_set_blk_dev_with_part(desc,
					plat->phandlepart.partition);
			} else {
				debug("%s: No device found\n", __func__);
				return -ENODEV;
			}
		}
	} else if (plat->mtdpart && plat->ubivol) {
		ret = mount_ubifs(plat->mtdpart, plat->ubivol);
		if (ret)
			return ret;

		ret = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
	} else {
		debug("Error: unsupported storage device.\n");
		return -ENODEV;
	}

	if (ret)
		debug("Error: could not access storage.\n");

	return ret;
}

static int fw_get_filesystem_prepare(struct udevice *dev)
{
	char *storage_interface, *dev_part, *ubi_mtdpart, *ubi_volume;
	struct device_plat *plat = dev_get_uclass_plat(dev);
	int ret;

	storage_interface = env_get("storage_interface");
	dev_part = env_get("fw_dev_part");
	ubi_mtdpart = env_get("fw_ubi_mtdpart");
	ubi_volume = env_get("fw_ubi_volume");

	if (storage_interface && dev_part) {
		ret = fs_set_blk_dev(storage_interface, dev_part, FS_TYPE_ANY);
	} else if (storage_interface && ubi_mtdpart && ubi_volume) {
		ret = mount_ubifs(ubi_mtdpart, ubi_volume);
		if (ret)
			return ret;

		if (!strcmp("ubi", storage_interface))
			ret = fs_set_blk_dev(storage_interface, NULL,
					     FS_TYPE_UBIFS);
		else
			ret = -ENODEV;
	} else {
		ret = select_fs_dev(plat);
	}

	return ret;
}

static void fw_get_filesystem_release(struct udevice *dev)
{
#ifdef CONFIG_CMD_UBIFS
	umount_ubifs();
#endif
}

/**
 * fw_get_filesystem_firmware - load firmware into an allocated buffer.
 * @dev: An instance of a driver.
 *
 * Return: Size of total read, negative value when error.
 */
static int fw_get_filesystem_firmware(struct udevice *dev)
{
	struct firmware *upriv = dev_get_uclass_priv(dev);
	loff_t actread = 0;
	int ret;

	ret = fw_get_filesystem_prepare(dev);
	if (ret)
		goto out;

	ret = fs_read(upriv->name, (ulong)map_to_sysmem(upriv->data),
		      upriv->offset, upriv->size, &actread);

	if (ret) {
		debug("Error: %d Failed to read %s from flash %lld != %zu.\n",
		      ret, upriv->name, actread, upriv->size);
	} else {
		ret = actread;
	}

out:
	fw_get_filesystem_release(dev);
	return ret;
}

/**
 * fw_get_filesystem_firmware_size - get firmware size.
 * @dev: An instance of a driver.
 *
 * Return: Size of firmware, negative value when error.
 */
static int fw_get_filesystem_firmware_size(struct udevice *dev)
{
	struct firmware *upriv = dev_get_uclass_priv(dev);
	loff_t size = 0;
	int ret;

	ret = fw_get_filesystem_prepare(dev);
	if (ret)
		goto out;

	ret = fs_size(upriv->name, &size);
	if (ret) {
		debug("Error: %d Failed to get size for %s.\n",
		      ret, upriv->name);
		goto out;
	}

	ret = size;

out:
	fw_get_filesystem_release(dev);
	return ret;
}

static const struct fw_loader_ops fs_loader_ops = {
	.get_firmware = fw_get_filesystem_firmware,
	.get_size = fw_get_filesystem_firmware_size,
};

static const struct udevice_id fs_loader_ids[] = {
	{ .compatible = "u-boot,fs-loader"},
	{ }
};

U_BOOT_DRIVER(fs_loader) = {
	.name			= "fs-loader",
	.id			= UCLASS_FIRMWARE_LOADER,
	.of_match		= fs_loader_ids,
	.ops			= &fs_loader_ops,
};

static struct device_plat default_plat = { 0 };

int get_fs_loader(struct udevice **dev)
{
	int ret;
	ofnode node = ofnode_get_chosen_node("firmware-loader");

	if (ofnode_valid(node))
		return uclass_get_device_by_ofnode(UCLASS_FIRMWARE_LOADER,
						   node, dev);

	/* Try the first device if none was chosen */
	ret = uclass_first_device_err(UCLASS_FIRMWARE_LOADER, dev);
	if (ret != -ENODEV)
		return ret;

	/* Just create a new device */
	ret = device_bind(dm_root(), DM_DRIVER_REF(fs_loader), "default-loader",
			  &default_plat, ofnode_null(), dev);
	if (ret)
		return ret;

	return device_probe(*dev);
}
