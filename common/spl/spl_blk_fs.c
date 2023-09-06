// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#include <common.h>
#include <spl.h>
#include <image.h>
#include <fs.h>

struct blk_dev {
	const char *ifname;
	char dev_part_str[8];
};

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	loff_t actlen;
	int ret;
	struct blk_dev *dev = (struct blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		return ret;
	}

	ret = fs_read(load->filename, (ulong)buf, file_offset, size, &actlen);
	if (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       load->filename, ret);
		return ret;
	}

	return actlen;
}

int spl_blk_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       enum uclass_id uclass_id, int devnum, int partnum)
{
	const char *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	struct legacy_img_hdr *header;
	struct blk_desc *blk_desc;
	loff_t actlen, filesize;
	struct blk_dev dev;
	int ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	if (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		return -ENODEV;
	}

	blk_show_device(uclass_id, devnum);
	header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));

	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, sizeof(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		goto out;
	}

	ret = fs_read(filename, (ulong)header, 0,
		      sizeof(struct legacy_img_hdr), &actlen);
	if (ret) {
		printf("spl: unable to read file %s. Err - %d\n", filename,
		       ret);
		goto out;
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT\n");
		load.read = spl_fit_read;
		load.bl_len = 1;
		load.filename = (void *)filename;
		load.priv = &dev;

		return spl_load_simple_fit(spl_image, &load, 0, header);
	}

	ret = spl_parse_image_header(spl_image, bootdev, header);
	if (ret) {
		printf("spl: unable to parse image header. Err - %d\n",
		       ret);
		goto out;
	}

	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		goto out;
	}

	ret = fs_size(filename, &filesize);
	if (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		goto out;
	}

	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		goto out;
	}

	ret = fs_read(filename, (ulong)spl_image->load_addr, 0, filesize,
		      &actlen);
	if (ret)
		printf("spl: unable to read file %s. Err - %d\n",
		       filename, ret);
out:
	return ret;
}
