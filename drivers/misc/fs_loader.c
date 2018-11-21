// SPDX-License-Identifier: GPL-2.0
 /*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 *
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <blk.h>
#include <fs.h>
#include <fs_loader.h>
#include <linux/string.h>
#include <mapmem.h>
#include <malloc.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

struct firmware_priv {
	const char *name;	/* Filename */
	u32 offset;		/* Offset of reading a file */
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

static int select_fs_dev(struct device_platdata *plat)
{
	int ret;

	if (plat->phandlepart.phandle) {
		ofnode node;

		node = ofnode_get_by_phandle(plat->phandlepart.phandle);

		int of_offset = ofnode_to_offset(node);

		struct udevice *dev;

		ret = device_get_global_by_of_offset(of_offset, &dev);
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
 * @name: Name of firmware file.
 * @dbuf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 * @firmwarep: Pointer to pointer to firmware image.
 *
 * Return: Negative value if fail, 0 for successful.
 */
static int _request_firmware_prepare(const char *name, void *dbuf,
				    size_t size, u32 offset,
				    struct firmware **firmwarep)
{
	if (!name || name[0] == '\0')
		return -EINVAL;

	/* No memory allocation is required if *firmwarep is allocated */
	if (!(*firmwarep)) {
		(*firmwarep) = calloc(1, sizeof(struct firmware));
		if (!(*firmwarep))
			return -ENOMEM;

		(*firmwarep)->priv = calloc(1, sizeof(struct firmware_priv));
		if (!(*firmwarep)->priv) {
			free(*firmwarep);
			return -ENOMEM;
		}
	} else if (!(*firmwarep)->priv) {
		(*firmwarep)->priv = calloc(1, sizeof(struct firmware_priv));
		if (!(*firmwarep)->priv) {
			free(*firmwarep);
			return -ENOMEM;
		}
	}

	((struct firmware_priv *)((*firmwarep)->priv))->name = name;
	((struct firmware_priv *)((*firmwarep)->priv))->offset = offset;
	(*firmwarep)->data = dbuf;
	(*firmwarep)->size = size;

	return 0;
}

/**
 * release_firmware - Release the resource associated with a firmware image
 * @firmware: Firmware resource to release
 */
void release_firmware(struct firmware *firmware)
{
	if (firmware) {
		if (firmware->priv) {
			free(firmware->priv);
			firmware->priv = NULL;
		}
		free(firmware);
	}
}

/**
 * fw_get_filesystem_firmware - load firmware into an allocated buffer.
 * @plat: Platform data such as storage and partition firmware loading from.
 * @firmware: pointer to firmware image.
 *
 * Return: Size of total read, negative value when error.
 */
static int fw_get_filesystem_firmware(struct device_platdata *plat,
				     struct firmware *firmware)
{
	struct firmware_priv *fw_priv = NULL;
	loff_t actread;
	char *storage_interface, *dev_part, *ubi_mtdpart, *ubi_volume;
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

	if (ret)
		goto out;

	fw_priv = firmware->priv;

	ret = fs_read(fw_priv->name, (ulong)map_to_sysmem(firmware->data),
			fw_priv->offset, firmware->size, &actread);
	if (ret) {
		debug("Error: %d Failed to read %s from flash %lld != %d.\n",
		      ret, fw_priv->name, actread, firmware->size);
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
 * @plat: Platform data such as storage and partition firmware loading from.
 * @name: Name of firmware file.
 * @buf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 * @firmwarep: Pointer to firmware image.
 *
 * The firmware is loaded directly into the buffer pointed to by @buf and
 * the @firmwarep data member is pointed at @buf.
 *
 * Return: Size of total read, negative value when error.
 */
int request_firmware_into_buf(struct device_platdata *plat,
			      const char *name,
			      void *buf, size_t size, u32 offset,
			      struct firmware **firmwarep)
{
	int ret;

	if (!plat)
		return -EINVAL;

	ret = _request_firmware_prepare(name, buf, size, offset, firmwarep);
	if (ret < 0) /* error */
		return ret;

	ret = fw_get_filesystem_firmware(plat, *firmwarep);

	return ret;
}

static int fs_loader_ofdata_to_platdata(struct udevice *dev)
{
	const char *fs_loader_path;
	u32 phandlepart[2];

	fs_loader_path = ofnode_get_chosen_prop("firmware-loader");

	if (fs_loader_path) {
		ofnode fs_loader_node;

		fs_loader_node = ofnode_path(fs_loader_path);
		if (ofnode_valid(fs_loader_node)) {
			struct device_platdata *plat;
			plat = dev->platdata;

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
		}
	}

	return 0;
}

static int fs_loader_probe(struct udevice *dev)
{
	return 0;
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
	.ofdata_to_platdata	= fs_loader_ofdata_to_platdata,
	.platdata_auto_alloc_size	= sizeof(struct device_platdata),
};

UCLASS_DRIVER(fs_loader) = {
	.id		= UCLASS_FS_FIRMWARE_LOADER,
	.name		= "fs-loader",
};
