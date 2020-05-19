// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <image.h>
#include <log.h>
#include <spl.h>

static ulong spl_nor_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, (void *)sector, count);

	return count;
}

unsigned long __weak spl_nor_get_uboot_base(void)
{
	return CONFIG_SYS_UBOOT_BASE;
}

static int spl_nor_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	__maybe_unused const struct image_header *header;
	__maybe_unused struct spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		/*
		 * Load Linux from its location in NOR flash to its defined
		 * location in SDRAM
		 */
		header = (const struct image_header *)CONFIG_SYS_OS_BASE;
#ifdef CONFIG_SPL_LOAD_FIT
		if (image_get_magic(header) == FDT_MAGIC) {
			int ret;

			debug("Found FIT\n");
			load.bl_len = 1;
			load.read = spl_nor_load_read;

			ret = spl_load_simple_fit(spl_image, &load,
						  CONFIG_SYS_OS_BASE,
						  (void *)header);

#if defined CONFIG_SYS_SPL_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
			memcpy((void *)CONFIG_SYS_SPL_ARGS_ADDR,
			       (void *)CONFIG_CMD_SPL_NOR_OFS,
			       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
			return ret;
		}
#endif
		if (image_get_os(header) == IH_OS_LINUX) {
			/* happy - was a Linux */
			int ret;

			ret = spl_parse_image_header(spl_image, header);
			if (ret)
				return ret;

			memcpy((void *)spl_image->load_addr,
			       (void *)(CONFIG_SYS_OS_BASE +
					sizeof(struct image_header)),
			       spl_image->size);
#ifdef CONFIG_SYS_FDT_BASE
			spl_image->arg = (void *)CONFIG_SYS_FDT_BASE;
#endif

			return 0;
		} else {
			puts("The Expected Linux image was not found.\n"
			     "Please check your NOR configuration.\n"
			     "Trying to start u-boot now...\n");
		}
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
#ifdef CONFIG_SPL_LOAD_FIT
	header = (const struct image_header *)spl_nor_get_uboot_base();
	if (image_get_magic(header) == FDT_MAGIC) {
		debug("Found FIT format U-Boot\n");
		load.bl_len = 1;
		load.read = spl_nor_load_read;
		return spl_load_simple_fit(spl_image, &load,
					   spl_nor_get_uboot_base(),
					   (void *)header);
	}
#endif
	if (IS_ENABLED(CONFIG_SPL_LOAD_IMX_CONTAINER)) {
		load.bl_len = 1;
		load.read = spl_nor_load_read;
		return spl_load_imx_container(spl_image, &load,
					      spl_nor_get_uboot_base());
	}

	/* Legacy image handling */
	if (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_SUPPORT)) {
		load.bl_len = 1;
		load.read = spl_nor_load_read;
		return spl_load_legacy_img(spl_image, &load,
					   spl_nor_get_uboot_base());
	}

	return 0;
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
