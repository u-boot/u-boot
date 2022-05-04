// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <dfu.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <malloc.h>
#include <mtd.h>
#include <uuid.h>
#include <vsprintf.h>

#include <dm/ofnode.h>

int fwu_get_mtd_alt_num(efi_guid_t *image_id, int *alt_num,
			const char *mtd_dev, bool guid)
{
	int i, nalt;
	int ret = -1;
	struct mtd_info *mtd;
	struct dfu_entity *dfu;
	ofnode node, parts_node;
	fdt_addr_t offset, size;
	char uuidbuf[UUID_STR_LEN + 1];

	mtd_probe_devices();
	mtd = get_mtd_device_nm(mtd_dev);

	/* Find partition node under given MTD device. */
	parts_node = ofnode_by_compatible(mtd_get_ofnode(mtd),
					  "fixed-partitions");

	uuid_bin_to_str(image_id->b, uuidbuf,
			guid ? UUID_STR_FORMAT_GUID : UUID_STR_FORMAT_STD);
	node = ofnode_by_prop_value(parts_node, "uuid", uuidbuf,
				    sizeof(uuidbuf));
	if (!ofnode_valid(node)) {
		log_warning("Warning: Failed to find partition by image UUID\n");
		return -ENOENT;
	}

	offset = ofnode_get_addr_size_index_notrans(node, 0, &size);
	if (offset == FDT_ADDR_T_NONE || !size)
		return -ENOENT;

	dfu_init_env_entities(NULL, NULL);

	nalt = 0;
	list_for_each_entry(dfu, &dfu_list, list) {
		nalt++;
	}

	if (!nalt) {
		log_warning("No entities in dfu_alt_info\n");
		dfu_free_entities();
		return -ENOENT;
	}

	for (i = 0; i < nalt; i++) {
		dfu = dfu_get_entity(i);

		if (!dfu)
			continue;

		if (dfu->dev_type != DFU_DEV_MTD)
			continue;

		if (dfu->layout == DFU_RAW_ADDR &&
		    dfu->data.mtd.start == offset &&
		    dfu->data.mtd.size == size) {
			*alt_num = dfu->alt;
			ret = 0;
			break;
		}
	}

	dfu_free_entities();

	return ret;
}

int gen_image_alt_info(char *buf, size_t len, int sidx,
		       struct fwu_image_entry *img, struct mtd_info *mtd)
{
	char *p = buf, *end = buf + len;
	char uuidbuf[UUID_STR_LEN + 1];
	ofnode node, parts_node;
	const char *suuid;
	int i;

	/* Find partition node under given MTD device. */
	parts_node = ofnode_by_compatible(mtd_get_ofnode(mtd),
					  "fixed-partitions");
	if (!ofnode_valid(parts_node))
		return -ENOENT;

	/* Check the media UUID if exist. */
	suuid = ofnode_read_string(parts_node, "uuid");
	if (suuid) {
		log_debug("Get location uuid %s\n", suuid);
		uuid_bin_to_str(img->location_uuid.b, uuidbuf,
				UUID_STR_FORMAT_STD);
		if (strcasecmp(suuid, uuidbuf))
			log_warning("Warning: Location UUID does not match!\n");
	}

	p += snprintf(p, end - p, "mtd %s", mtd->name);
	if (end < p)
		return -E2BIG;

	/*
	 * List up the image banks in the FWU mdata and search the corresponding
	 * partition based on partition's uuid.
	 */
	for (i = 0; i < CONFIG_FWU_NUM_BANKS; i++) {
		struct fwu_image_bank_info *bank;
		fdt_addr_t offset, size;

		/* Query a partition by image UUID */
		bank = &img->img_bank_info[i];
		uuid_bin_to_str(bank->image_uuid.b, uuidbuf,
				UUID_STR_FORMAT_STD);
		node = ofnode_by_prop_value(parts_node, "uuid", uuidbuf,
					    sizeof(uuidbuf));
		if (!ofnode_valid(node)) {
			log_warning("Warning: Failed to find partition by image UUID\n");
			break;
		}

		offset = ofnode_get_addr_size_index_notrans(node, 0, &size);
		if (offset == FDT_ADDR_T_NONE || !size)
			break;

		p += snprintf(p, end - p, "%sbank%d raw %lx %lx",
			      i == 0 ? "=" : ";", i, (unsigned long)offset,
			      (unsigned long)size);
		if (end < p)
			return -E2BIG;
	}

	return i != CONFIG_FWU_NUM_BANKS ? -ENOENT : 0;
}

int fwu_gen_alt_info_from_mtd(char *buf, size_t len, struct mtd_info *mtd)
{
	struct fwu_mdata *mdata;
	int i, l, ret;

	ret = fwu_get_mdata(&mdata);
	if (ret < 0) {
		log_debug("Failed to get the FWU mdata.\n");
		return ret;
	}

	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		ret = gen_image_alt_info(buf, len, i * CONFIG_FWU_NUM_BANKS,
					 &mdata->img_entry[i], mtd);
		if (ret)
			break;
		l = strlen(buf);
		/* Replace the last ';' with '&' if there is another image. */
		if (i != CONFIG_FWU_NUM_IMAGES_PER_BANK - 1 && l)
			buf[l - 1] = '&';
		len -= l;
		buf += l;
	}

	free(mdata);

	return ret;
}
