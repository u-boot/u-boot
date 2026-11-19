// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 *
 */

#define LOG_CATEGORY UCLASS_FIRMWARE_LOADER

#include <dm.h>
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

static int mount_fs_dev(struct fw_loader_plat *plat)
{
	if (plat->phandlepart.phandle)
		return 0;
	else if (plat->mtdpart && plat->ubivol)
		return mount_ubifs(plat->mtdpart, plat->ubivol);
	else
		return 0;
}

static int select_fs_dev(struct fw_loader_plat *plat)
{
	int ret;

	if (plat->phandlepart.phandle) {
		struct blk_desc *desc;

		ret = generic_fw_loader_get_phandle_desc(plat, &desc);
		if (ret)
			debug("%s: No device found\n", __func__);
		else
			ret = fs_set_blk_dev_with_part(desc,
				plat->phandlepart.partition);
	} else if (plat->mtdpart && plat->ubivol) {
		ret = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
	} else {
		debug("Error: unsupported storage device.\n");
		return -ENODEV;
	}

	if (ret)
		debug("Error: could not access storage.\n");

	return ret;
}

static int fw_get_filesystem_mount(struct udevice *dev)
{
	struct fw_loader_plat *plat = dev_get_uclass_plat(dev);
	struct fw_loader_env_info info = { };

	generic_fw_loader_env_info(&info);

	if (info.storage_interface && info.dev_part)
		return 0;
	else if (info.storage_interface && info.ubi_mtdpart &&
		 info.ubi_volume)
		return mount_ubifs(info.ubi_mtdpart, info.ubi_volume);
	else
		return mount_fs_dev(plat);
}

static int fw_get_filesystem_prepare(struct udevice *dev)
{
	struct fw_loader_plat *plat = dev_get_uclass_plat(dev);
	struct fw_loader_env_info info = { };
	int ret;

	generic_fw_loader_env_info(&info);

	if (info.storage_interface && info.dev_part) {
		ret = fs_set_blk_dev(info.storage_interface,
				     info.dev_part, FS_TYPE_ANY);
	} else if (info.storage_interface && info.ubi_mtdpart &&
		   info.ubi_volume) {
		if (!strcmp("ubi", info.storage_interface))
			ret = fs_set_blk_dev(info.storage_interface, NULL,
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

static int __fw_get_filesystem_firmware_size(struct udevice *dev,
					     const char *name, loff_t *size)
{
	int ret;

	ret = fw_get_filesystem_prepare(dev);
	if (ret)
		return ret;

	return fs_size(name, size);
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
	loff_t size, actread = 0;
	int ret;

	ret = fw_get_filesystem_mount(dev);
	if (ret)
		goto out;

	ret = __fw_get_filesystem_firmware_size(dev, upriv->name, &size);
	if (ret)
		goto out;

	if (upriv->offset > size) {
		log_err("Offset %#x beyond file size %#llx\n",
			upriv->offset, size);
		ret = -EINVAL;
		goto out;
	}

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
	loff_t size;
	int ret;

	ret = fw_get_filesystem_mount(dev);
	if (ret)
		goto out;

	ret = __fw_get_filesystem_firmware_size(dev, upriv->name, &size);
	if (ret)
		goto out;

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

static struct fw_loader_plat default_plat = { 0 };

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
