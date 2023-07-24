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
	const char *filename = CONFIG_SPL_PAYLOAD;
	struct disk_partition part_info = {};
	struct blk_desc *blk_desc;
	loff_t filesize;
	struct blk_dev dev;
	int ret;
	struct spl_load_info load = {
		.read = spl_fit_read,
		.bl_len = 1,
		.filename = filename,
		.priv = &dev,
	};

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	if (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		return ret;
	}

	blk_show_device(uclass_id, devnum);
	ret = part_get_info(blk_desc, 1, &part_info);
	if (ret) {
		printf("spl: no partition table found. Err - %d\n", ret);
		return ret;
	}

	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, sizeof(dev.dev_part_str) - 1, "%d:%d",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		return ret;
	}

	ret = fs_size(filename, &filesize);
	if (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		return ret;
	}

	return spl_load(spl_image, bootdev, &load, filesize, 0);
}
