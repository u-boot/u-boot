// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <squashfs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	char *filename = load->priv;
	loff_t actread;
	int ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		if (image_get_magic(header) != FDT_MAGIC)
			return size;
	}

	return actread;
}

int spl_load_image_sqfs(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev,
			struct blk_desc *block_dev, int partition,
			const char *filename)
{
	int err;
	loff_t size = 0;
	struct spl_load_info load;
	struct disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	if (err) {
		printf("spl: no partition table found\n");
		goto end;
	}

	err = sqfs_probe(block_dev, &part_info);
	if (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		goto end;
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		if (err)
			goto end;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	if (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	return err;
}
