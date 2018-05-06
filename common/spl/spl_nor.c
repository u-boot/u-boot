// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <spl.h>

static int spl_nor_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int ret;
	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		const struct image_header *header;

		/*
		 * Load Linux from its location in NOR flash to its defined
		 * location in SDRAM
		 */
		header = (const struct image_header *)CONFIG_SYS_OS_BASE;

		if (image_get_os(header) == IH_OS_LINUX) {
			/* happy - was a Linux */

			ret = spl_parse_image_header(spl_image, header);
			if (ret)
				return ret;

			memcpy((void *)spl_image->load_addr,
			       (void *)(CONFIG_SYS_OS_BASE +
					sizeof(struct image_header)),
			       spl_image->size);

			spl_image->arg = (void *)CONFIG_SYS_FDT_BASE;

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
	ret = spl_parse_image_header(spl_image,
			(const struct image_header *)CONFIG_SYS_UBOOT_BASE);
	if (ret)
		return ret;

	memcpy((void *)(unsigned long)spl_image->load_addr,
	       (void *)(CONFIG_SYS_UBOOT_BASE + sizeof(struct image_header)),
	       spl_image->size);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
