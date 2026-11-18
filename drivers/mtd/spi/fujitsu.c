// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info fujitsu_nor_parts[] = {
	{
		INFO_NAME("mb85rs256ty")
		.id = {0x04, 0x7f, 0x25, 0x00, 0x00},
		.id_len = 3,
		.sector_size = 32 * 1024,
		.n_sectors = 1,
		.flags = SPI_NOR_NO_ERASE,
		.addr_width = 2,
	},
};

#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
static void fujitsu_late_init(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	/* Whole chip can be written at once */
	params->page_size = params->size;
}

static struct spi_nor_fixups fujitsu_fixups = {
	.late_init = fujitsu_late_init,
};

static void fujitsu_set_fixups(struct spi_nor *nor)
{
	nor->fixups = &fujitsu_fixups;
}
#endif /* !CONFIG_IS_ENABLED(SPI_FLASH_TINY) */

const struct spi_nor_manufacturer spi_nor_fujitsu = {
	.parts = fujitsu_nor_parts,
	.nparts = ARRAY_SIZE(fujitsu_nor_parts),
#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
	.set_fixups = fujitsu_set_fixups,
#endif
};
