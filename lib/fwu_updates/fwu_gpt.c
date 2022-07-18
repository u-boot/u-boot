// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <blk.h>
#include <dfu.h>
#include <efi.h>
#include <efi_loader.h>
#include <fwu.h>
#include <log.h>
#include <part.h>

#include <linux/errno.h>

static int get_gpt_dfu_identifier(struct blk_desc *desc, efi_guid_t *image_guid)
{
	int i;
	struct disk_partition info;
	efi_guid_t unique_part_guid;

	for (i = 1; i < MAX_SEARCH_PARTITIONS; i++) {
		if (part_get_info(desc, i, &info))
			continue;
		uuid_str_to_bin(info.uuid, unique_part_guid.b,
				UUID_STR_FORMAT_GUID);

		if (!guidcmp(&unique_part_guid, image_guid))
			return i;
	}

	log_err("No partition found with image_guid %pUs\n", image_guid);
	return -ENOENT;
}

static int fwu_gpt_get_alt_num(struct blk_desc *desc, efi_guid_t *image_guid,
			       u8 *alt_num, unsigned char dfu_dev)
{
	int ret = -1;
	int i, part, dev_num;
	int nalt;
	struct dfu_entity *dfu;

	dev_num = desc->devnum;
	part = get_gpt_dfu_identifier(desc, image_guid);
	if (part < 0)
		return -ENOENT;

	dfu_init_env_entities(NULL, NULL);

	nalt = 0;
	list_for_each_entry(dfu, &dfu_list, list)
		nalt++;

	if (!nalt) {
		log_warning("No entities in dfu_alt_info\n");
		dfu_free_entities();
		return -ENOENT;
	}

	for (i = 0; i < nalt; i++) {
		dfu = dfu_get_entity(i);

		if (!dfu)
			continue;

		/*
		 * Currently, Multi Bank update
		 * feature is being supported
		 * only on GPT partitioned
		 * MMC/SD devices.
		 */
		if (dfu->dev_type != dfu_dev)
			continue;

		if (dfu->layout == DFU_RAW_ADDR &&
		    dfu->data.mmc.dev_num == dev_num &&
		    dfu->data.mmc.part == part) {
			*alt_num = dfu->alt;
			ret = 0;
			break;
		}
	}

	dfu_free_entities();

	return ret;
}

/**
 * fwu_plat_get_alt_num() - Get the DFU alt number
 * @dev: FWU metadata device
 * @image_guid: GUID value of the image for which the alt num is to
 *              be obtained
 * @alt_num: The DFU alt number for the image that is to be updated
 *
 * Get the DFU alt number for the image that is to be updated. The
 * image is identified with the image_guid parameter that is passed
 * to the function.
 *
 * Note: This is a weak function and platforms can override this with
 * their own implementation for obtaining the alt number value.
 *
 * Return: 0 if OK, -ve on error
 *
 */
__weak int fwu_plat_get_alt_num(struct udevice *dev, efi_guid_t *image_guid,
				u8 *alt_num)
{
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	return fwu_gpt_get_alt_num(dev_get_uclass_plat(priv->blk_dev),
				   image_guid, alt_num, DFU_DEV_MMC);
}
