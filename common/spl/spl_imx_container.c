// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#include <common.h>
#include <stdlib.h>
#include <errno.h>
#include <imx_container.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>
#ifdef CONFIG_AHAB_BOOT
#include <asm/mach-imx/ahab.h>
#endif

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
		       map_sysmem(images[image_index].dst,
				  images[image_index].size)) != sectors) {
		printf("%s wrong\n", __func__);
		return NULL;
	}

#ifdef CONFIG_AHAB_BOOT
	if (ahab_verify_cntr_image(&images[image_index], image_index))
		return NULL;
#endif

	return &images[image_index];
}

static int read_auth_container(struct spl_image_info *spl_image,
			       struct spl_load_info *info, ulong sector)
{
	struct container_hdr *container = NULL;
	u16 length;
	u32 sectors;
	int i, size, ret = 0;

	size = roundup(CONTAINER_HDR_ALIGNMENT, info->bl_len);
	sectors = size / info->bl_len;

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	if (!container)
		return -ENOMEM;

	debug("%s: container: %p sector: %lu sectors: %u\n", __func__,
	      container, sector, sectors);
	if (info->read(info, sector, sectors, container) != sectors) {
		ret = -EIO;
		goto end;
	}

	if (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		goto end;
	}

	if (!container->num_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		goto end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	if (length > CONTAINER_HDR_ALIGNMENT) {
		size = roundup(length, info->bl_len);
		sectors = size / info->bl_len;

		free(container);
		container = malloc(size);
		if (!container)
			return -ENOMEM;

		debug("%s: container: %p sector: %lu sectors: %u\n",
		      __func__, container, sector, sectors);
		if (info->read(info, sector, sectors, container) !=
		    sectors) {
			ret = -EIO;
			goto end;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	ret = ahab_auth_cntr_hdr(container, length);
	if (ret)
		goto end_auth;
#endif

	for (i = 0; i < container->num_images; i++) {
		struct boot_img_t *image = read_auth_image(spl_image, info,
							   container, i,
							   sector);

		if (!image) {
			ret = -EINVAL;
			goto end_auth;
		}

		if (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	free(container);

	return ret;
}

int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong sector)
{
	return read_auth_container(spl_image, info, sector);
}
