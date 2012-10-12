/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <spl.h>

void spl_nor_load_image(void)
{
	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image.flags |= SPL_COPY_PAYLOAD_ONLY;

	if (spl_start_uboot()) {
		/*
		 * Load real U-Boot from its location in NOR flash to its
		 * defined location in SDRAM
		 */
		spl_parse_image_header(
			(const struct image_header *)CONFIG_SYS_UBOOT_BASE);

		memcpy((void *)spl_image.load_addr,
		       (void *)(CONFIG_SYS_UBOOT_BASE +
				sizeof(struct image_header)),
		       spl_image.size);
	} else {
		/*
		 * Load Linux from its location in NOR flash to its defined
		 * location in SDRAM
		 */
		spl_parse_image_header(
			(const struct image_header *)CONFIG_SYS_OS_BASE);

		memcpy((void *)spl_image.load_addr,
		       (void *)(CONFIG_SYS_OS_BASE +
				sizeof(struct image_header)),
		       spl_image.size);

		/*
		 * Copy DT blob (fdt) to SDRAM. Passing pointer to flash
		 * doesn't work (16 KiB should be enough for DT)
		 */
		memcpy((void *)CONFIG_SYS_SPL_ARGS_ADDR,
		       (void *)(CONFIG_SYS_FDT_BASE),
		       (16 << 10));
	}
}
