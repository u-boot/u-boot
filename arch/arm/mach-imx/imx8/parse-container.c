// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/image.h>

static struct boot_img_t *read_auth_image(struct spl_image_info *spl_image,
					  struct spl_load_info *info,
					  struct container_hdr *container,
					  int image_index,
					  u32 container_sector)
{
	struct boot_img_t *images;
	ulong sector;
	u32 sectors;

	if (image_index > container->num_images) {
		debug("Invalid image number\n");
		return NULL;
	}

	images = (struct boot_img_t *)((u8 *)container +
				       sizeof(struct container_hdr));

	if (images[image_index].offset % info->bl_len) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, info->bl_len);
		return NULL;
	}

	sectors = roundup(images[image_index].size, info->bl_len) /
		info->bl_len;
	sector = images[image_index].offset / info->bl_len +
		container_sector;

	debug("%s: container: %p sector: %lu sectors: %u\n", __func__,
	      container, sector, sectors);
	if (info->read(info, sector, sectors,
		       (void *)images[image_index].entry) != sectors) {
		printf("%s wrong\n", __func__);
		return NULL;
	}

	return &images[image_index];
}

static int read_auth_container(struct spl_image_info *spl_image,
			       struct spl_load_info *info, ulong sector)
{
	struct container_hdr *container = NULL;
	u16 length;
	u32 sectors;
	int i, size;

	size = roundup(CONTAINER_HDR_ALIGNMENT, info->bl_len);
	sectors = size / info->bl_len;

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = (struct container_hdr *)spl_get_load_buffer(-size, size);

	debug("%s: container: %p sector: %lu sectors: %u\n", __func__,
	      container, sector, sectors);
	if (info->read(info, sector, sectors, container) != sectors)
		return -EIO;

	if (container->tag != 0x87 && container->version != 0x0) {
		printf("Wrong container header");
		return -ENOENT;
	}

	if (!container->num_images) {
		printf("Wrong container, no image found");
		return -ENOENT;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	if (length > CONTAINER_HDR_ALIGNMENT) {
		size = roundup(length, info->bl_len);
		sectors = size / info->bl_len;

		container = (struct container_hdr *)spl_get_load_buffer(-size, size);

		debug("%s: container: %p sector: %lu sectors: %u\n",
		      __func__, container, sector, sectors);
		if (info->read(info, sector, sectors, container) !=
		    sectors)
			return -EIO;
	}

	for (i = 0; i < container->num_images; i++) {
		struct boot_img_t *image = read_auth_image(spl_image, info,
							   container, i,
							   sector);

		if (!image)
			return -EINVAL;

		if (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

	return 0;
}

int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong sector)
{
	return read_auth_container(spl_image, info, sector);
}
