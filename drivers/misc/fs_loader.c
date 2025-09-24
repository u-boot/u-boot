// SPDX-License-Identifier: GPL-2.0+
 /*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#define LOG_CATEGORY UCLASS_FS_FIRMWARE_LOADER

#include <dm.h>
#include <env.h>
#include <errno.h>
#include <blk.h>
#include <fs.h>
#include <fs_loader.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <linux/string.h>
#include <mapmem.h>
#include <malloc.h>
#include <spl.h>
#include <spi_flash.h>

#ifdef CONFIG_CMD_UBIFS
#include <ubi_uboot.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct firmware - A place for storing firmware and its attribute data.
 *
 * This holds information about a firmware and its content.
 *
 * @size: Size of a file
 * @data: Buffer for file
 * @priv: Firmware loader private fields
 * @name: Filename
 * @offset: Offset of reading a file
 */
struct firmware {
	size_t size;
	const u8 *data;
	const char *name;
	u32 offset;
};

#ifdef CONFIG_CMD_UBIFS
static int mount_ubifs(char *mtdpart, char *ubivol)
{
	int ret = ubi_part(mtdpart, NULL);

	if (ret) {
		debug("Cannot find mtd partition %s\n", mtdpart);
		return ret;
	}

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

__weak struct blk_desc *blk_get_by_device(struct udevice *dev)
{
	return NULL;
}

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
	struct firmware *firmwarep = dev_get_priv(dev);
	struct device_plat *plat = dev_get_plat(dev);
	char *endptr;
	u32 fw_offset;

	if (!firmwarep)
		return -ENOMEM;

	firmwarep->name = name;

	if (plat->data_type == DATA_RAW) {
		fw_offset = simple_strtoul(firmwarep->name, &endptr, 16);
		if (firmwarep->name == endptr || *endptr != '\0')
			return -EINVAL;

		firmwarep->offset = fw_offset + offset;
	} else {
		firmwarep->offset = offset;
	}

	firmwarep->data = dbuf;
	firmwarep->size = size;

	return 0;
}

/**
 * fw_get_filesystem_firmware - load firmware into an allocated buffer.
 * @dev: An instance of a driver.
 *
 * Return: Size of total read, negative value when error.
 */
static int fw_get_filesystem_firmware(struct udevice *dev)
{
	loff_t actread = 0;
	char *storage_interface, *dev_part, *ubi_mtdpart, *ubi_volume;
	int ret = 0;
	struct device_plat *plat = dev_get_plat(dev);

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
		if (plat->data_type == DATA_FS)
			ret = select_fs_dev(dev_get_plat(dev));
	}

	if (ret)
		goto out;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -ENOMEM;

	if (plat->data_type == DATA_FS)
		ret = fs_read(firmwarep->name,
			      (ulong)map_to_sysmem(firmwarep->data),
			      firmwarep->offset, firmwarep->size, &actread);
	else if (plat->data_type == DATA_RAW) {
#ifdef CONFIG_SPI_FLASH
		ret = spi_flash_read_dm(plat->flash, firmwarep->offset,
					firmwarep->size,
					(void *)firmwarep->data);
		actread = firmwarep->size;
#endif
	}

	if (ret) {
		debug("Error: %d Failed to read %s from flash %lld != %zu.\n",
		      ret, firmwarep->name, actread, firmwarep->size);
	} else {
		ret = actread;
	}

out:
#ifdef CONFIG_CMD_UBIFS
	umount_ubifs();
#endif
	return ret;
}

/**
 * request_firmware_into_buf - Load firmware into a previously allocated buffer.
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 * @buf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 *
 * The firmware is loaded directly into the buffer pointed to by @buf.
 *
 * Return: Size of total read, negative value when error.
 */
int request_firmware_into_buf(struct udevice *dev,
			      const char *name,
			      void *buf, size_t size, u32 offset)
{
	int ret;

	if (!dev)
		return -EINVAL;

	ret = _request_firmware_prepare(dev, name, buf, size, offset);
	if (ret < 0) /* error */
		return ret;

	ret = fw_get_filesystem_firmware(dev);

	return ret;
}

static int fs_loader_of_to_plat(struct udevice *dev)
{
	u32 phandlepart[2];
	u32 sfconfig[2];

	ofnode fs_loader_node = dev_ofnode(dev);

	if (ofnode_valid(fs_loader_node)) {
		struct device_plat *plat;

		plat = dev_get_plat(dev);
		if (!ofnode_read_u32_array(fs_loader_node,
					  "phandlepart",
					  phandlepart, 2)) {
			plat->phandlepart.phandle = phandlepart[0];
			plat->phandlepart.partition = phandlepart[1];
		}

		plat->mtdpart = (char *)ofnode_read_string(
				 fs_loader_node, "mtdpart");

		plat->ubivol = (char *)ofnode_read_string(
				 fs_loader_node, "ubivol");

		if (!ofnode_read_u32_array(fs_loader_node, "sfconfig",
					   sfconfig, 2)) {
			plat->data_type = DATA_RAW;
			log_warning("%s: Chosen RAW data type DOESN'T have "
				    "built-in data integrity support\n",
				    __func__);
			plat->sfconfig.bus = sfconfig[0];
			plat->sfconfig.cs = sfconfig[1];
		} else {
			plat->data_type = DATA_FS;
		}
	}

	return 0;
}

static int fs_loader_probe(struct udevice *dev)
{
	int ret = 0;
	struct device_plat *plat = dev_get_plat(dev);

#ifdef CONFIG_SPI_FLASH
	if (!plat->flash) {
		debug("bus = %d\ncs = %d\n",
		      plat->sfconfig.bus, plat->sfconfig.cs);

		ret = spi_flash_probe_bus_cs(plat->sfconfig.bus,
					     plat->sfconfig.cs,
					     &plat->flash);
		if (ret) {
			debug("fs_loader: Failed to initialize SPI flash at ");
			debug("%u:%u (error %d)\n", plat->sfconfig.bus,
			      plat->sfconfig.cs, ret);
			return -ENODEV;
		}

		if (!plat->flash)
			return -EINVAL;
	}
#endif

	if (IS_ENABLED(CONFIG_DM) && IS_ENABLED(CONFIG_BLK)) {
		if (plat->phandlepart.phandle) {
			ofnode node = ofnode_get_by_phandle(plat->phandlepart.phandle);
			struct udevice *parent_dev = NULL;

			ret = device_get_global_by_ofnode(node, &parent_dev);
			if (!ret) {
				struct udevice *dev;

				ret = blk_get_from_parent(parent_dev, &dev);
				if (ret) {
					debug("fs_loader: No block device: %d\n",
					      ret);

					return ret;
				}
			}
		}
	}

	return ret;
};

static const struct udevice_id fs_loader_ids[] = {
	{ .compatible = "u-boot,fs-loader"},
	{ }
};

U_BOOT_DRIVER(fs_loader) = {
	.name			= "fs-loader",
	.id			= UCLASS_FS_FIRMWARE_LOADER,
	.of_match		= fs_loader_ids,
	.probe			= fs_loader_probe,
	.of_to_plat	= fs_loader_of_to_plat,
	.plat_auto	= sizeof(struct device_plat),
	.priv_auto	= sizeof(struct firmware),
};

static struct device_plat default_plat = { 0 };

int get_fs_loader(struct udevice **dev)
{
	int ret;
	ofnode node = ofnode_get_chosen_node("firmware-loader");

	if (ofnode_valid(node))
		return uclass_get_device_by_ofnode(UCLASS_FS_FIRMWARE_LOADER,
						   node, dev);

	/* Try the first device if none was chosen */
	ret = uclass_first_device_err(UCLASS_FS_FIRMWARE_LOADER, dev);
	if (ret != -ENODEV)
		return ret;

	/* Just create a new device */
	ret = device_bind(dm_root(), DM_DRIVER_REF(fs_loader), "default-loader",
			  &default_plat, ofnode_null(), dev);
	if (ret)
		return ret;

	return device_probe(*dev);
}

UCLASS_DRIVER(fs_loader) = {
	.id		= UCLASS_FS_FIRMWARE_LOADER,
	.name		= "fs-loader",
};
