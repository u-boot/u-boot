// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <spl.h>
#include <asm/mach-imx/image.h>
#ifdef CONFIG_AHAB_BOOT
#include <asm/arch/sci/sci.h>
#endif

#define SEC_SECURE_RAM_BASE		0x31800000UL
#define SEC_SECURE_RAM_END_BASE		(SEC_SECURE_RAM_BASE + 0xFFFFUL)
#define SECO_LOCAL_SEC_SEC_SECURE_RAM_BASE	0x60000000UL

#define SECO_PT         2U

#ifdef CONFIG_AHAB_BOOT
static int authenticate_image(struct boot_img_t *img, int image_index)
{
	sc_faddr_t start, end;
	sc_rm_mr_t mr;
	int err;
	int ret = 0;

	debug("img %d, dst 0x%x, src 0x%x, size 0x%x\n",
	      image_index, (uint32_t)img->dst, img->offset, img->size);

	/* Find the memreg and set permission for seco pt */
	err = sc_rm_find_memreg(-1, &mr,
				img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1),
				ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1);

	if (err) {
		printf("can't find memreg for image %d load address 0x%x, error %d\n",
		       image_index, img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1), err);
		return -ENOMEM;
	}

	err = sc_rm_get_memreg_info(-1, mr, &start, &end);
	if (!err)
		debug("memreg %u 0x%x -- 0x%x\n", mr, start, end);

	err = sc_rm_set_memreg_permissions(-1, mr,
					   SECO_PT, SC_RM_PERM_FULL);
	if (err) {
		printf("set permission failed for img %d, error %d\n",
		       image_index, err);
		return -EPERM;
	}

	err = sc_seco_authenticate(-1, SC_SECO_VERIFY_IMAGE,
				   1 << image_index);
	if (err) {
		printf("authenticate img %d failed, return %d\n",
		       image_index, err);
		ret = -EIO;
	}

	err = sc_rm_set_memreg_permissions(-1, mr,
					   SECO_PT, SC_RM_PERM_NONE);
	if (err) {
		printf("remove permission failed for img %d, error %d\n",
		       image_index, err);
		ret = -EPERM;
	}

	return ret;
}
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
		       (void *)images[image_index].entry) != sectors) {
		printf("%s wrong\n", __func__);
		return NULL;
	}

#ifdef CONFIG_AHAB_BOOT
	if (authenticate_image(&images[image_index], image_index)) {
		printf("Failed to authenticate image %d\n", image_index);
		return NULL;
	}
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

#ifdef CONFIG_AHAB_BOOT
	memcpy((void *)SEC_SECURE_RAM_BASE, (const void *)container,
	       ALIGN(length, CONFIG_SYS_CACHELINE_SIZE));

	ret = sc_seco_authenticate(-1, SC_SECO_AUTH_CONTAINER,
				   SECO_LOCAL_SEC_SEC_SECURE_RAM_BASE);
	if (ret) {
		printf("authenticate container hdr failed, return %d\n", ret);
		return ret;
	}
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
	if (sc_seco_authenticate(-1, SC_SECO_REL_CONTAINER, 0))
		printf("Error: release container failed!\n");
#endif
	return ret;
}

int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong sector)
{
	return read_auth_container(spl_image, info, sector);
}
