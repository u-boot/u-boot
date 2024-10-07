// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <dm.h>
#include <dfu.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <malloc.h>
#include <mtd.h>
#include <u-boot/uuid.h>
#include <stdio.h>

#include <dm/ofnode.h>

static struct fwu_mtd_image_info *mtd_img_by_uuid(const char *uuidbuf)
{
	int num_images;
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(fwu_get_dev());
	struct fwu_mtd_image_info *image_info = mtd_priv->fwu_mtd_images;

	if (!image_info)
		return NULL;

	num_images = CONFIG_FWU_NUM_BANKS *
		CONFIG_FWU_NUM_IMAGES_PER_BANK;

	for (int i = 0; i < num_images; i++)
		if (!strcmp(uuidbuf, image_info[i].uuidbuf))
			return &image_info[i];

	return NULL;
}

int fwu_mtd_get_alt_num(efi_guid_t *image_id, u8 *alt_num,
			const char *mtd_dev)
{
	struct fwu_mtd_image_info *mtd_img_info;
	char uuidbuf[UUID_STR_LEN + 1];
	fdt_addr_t offset, size = 0;
	struct dfu_entity *dfu;
	int i, nalt, ret;

	mtd_probe_devices();

	uuid_bin_to_str(image_id->b, uuidbuf, UUID_STR_FORMAT_STD);

	mtd_img_info = mtd_img_by_uuid(uuidbuf);
	if (!mtd_img_info) {
		log_err("%s: Not found partition for image %s\n", __func__, uuidbuf);
		return -ENOENT;
	}

	offset = mtd_img_info->start;
	size = mtd_img_info->size;

	ret = dfu_init_env_entities(NULL, NULL);
	if (ret)
		return -ENOENT;

	nalt = list_count_nodes(&dfu_list);
	if (!nalt) {
		log_warning("No entities in dfu_alt_info\n");
		dfu_free_entities();
		return -ENOENT;
	}

	ret = -ENOENT;
	for (i = 0; i < nalt; i++) {
		dfu = dfu_get_entity(i);

		/* Only MTD RAW access */
		if (!dfu || dfu->dev_type != DFU_DEV_MTD ||
		    dfu->layout != DFU_RAW_ADDR ||
			dfu->data.mtd.start != offset ||
			dfu->data.mtd.size != size)
			continue;

		*alt_num = dfu->alt;
		ret = 0;
		break;
	}

	dfu_free_entities();

	log_debug("%s: %s -> %d\n", __func__, uuidbuf, *alt_num);
	return ret;
}

/**
 * fwu_plat_get_alt_num() - Get the DFU Alt Num for the image from the platform
 * @dev: FWU device
 * @image_id: Image GUID for which DFU alt number needs to be retrieved
 * @alt_num: Pointer to the alt_num
 *
 * Get the DFU alt number from the platform for the image specified by the
 * image GUID.
 *
 * Note: This is a weak function and platforms can override this with
 * their own implementation for obtaining the alt number value.
 *
 * Return: 0 if OK, -ve on error
 */
__weak int fwu_plat_get_alt_num(struct udevice *dev, efi_guid_t *image_id,
				u8 *alt_num)
{
	return fwu_mtd_get_alt_num(image_id, alt_num, "nor1");
}

static int gen_image_alt_info(char *buf, size_t len,
			      struct fwu_image_entry *img, struct mtd_info *mtd)
{
	char *p = buf, *end = buf + len;
	int i;

	p += snprintf(p, end - p, "mtd %s", mtd->name);
	if (end < p) {
		log_err("%s:%d Run out of buffer\n", __func__, __LINE__);
		return -E2BIG;
	}

	/*
	 * List the image banks in the FWU mdata and search the corresponding
	 * partition based on partition's uuid.
	 */
	for (i = 0; i < CONFIG_FWU_NUM_BANKS; i++) {
		struct fwu_mtd_image_info *mtd_img_info;
		struct fwu_image_bank_info *bank;
		char uuidbuf[UUID_STR_LEN + 1];
		u32 offset, size;

		/* Query a partition by image UUID */
		bank = &img->img_bank_info[i];
		uuid_bin_to_str(bank->image_guid.b, uuidbuf, UUID_STR_FORMAT_STD);

		mtd_img_info = mtd_img_by_uuid(uuidbuf);
		if (!mtd_img_info) {
			log_err("%s: Not found partition for image %s\n", __func__, uuidbuf);
			break;
		}

		offset = mtd_img_info->start;
		size = mtd_img_info->size;

		p += snprintf(p, end - p, "%sbank%d raw %x %x",
			      i == 0 ? "=" : ";", i, offset, size);
		if (end < p) {
			log_err("%s:%d Run out of buffer\n", __func__, __LINE__);
			return -E2BIG;
		}
	}

	if (i == CONFIG_FWU_NUM_BANKS)
		return 0;

	return -ENOENT;
}

int fwu_gen_alt_info_from_mtd(char *buf, size_t len, struct mtd_info *mtd)
{
	int i, l, ret;
	struct fwu_data *data = fwu_get_data();
	struct fwu_image_entry *img_entry;

	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		img_entry = &data->fwu_images[i];
		ret = gen_image_alt_info(buf, len, img_entry, mtd);
		if (ret)
			break;

		l = strlen(buf);
		/* Replace the last ';' with '&' if there is another image. */
		if (i != CONFIG_FWU_NUM_IMAGES_PER_BANK - 1 && l) {
			buf[l] = '&';
			buf++;
		}
		len -= l;
		buf += l;
	}

	return ret;
}
