/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

int spl_nor_load_image(void)
{
	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image.flags |= SPL_COPY_PAYLOAD_ONLY;

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

			spl_parse_image_header(header);

			memcpy((void *)spl_image.load_addr,
			       (void *)(CONFIG_SYS_OS_BASE +
					sizeof(struct image_header)),
			       spl_image.size);

			/*
			 * Copy DT blob (fdt) to SDRAM. Passing pointer to
			 * flash doesn't work (16 KiB should be enough for DT)
			 */
			memcpy((void *)CONFIG_SYS_SPL_ARGS_ADDR,
			       (void *)(CONFIG_SYS_FDT_BASE),
			       (16 << 10));

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
	spl_parse_image_header(
			(const struct image_header *)CONFIG_SYS_UBOOT_BASE);

	memcpy((void *)(unsigned long)spl_image.load_addr,
	       (void *)(CONFIG_SYS_UBOOT_BASE + sizeof(struct image_header)),
	       spl_image.size);

	return 0;
}
