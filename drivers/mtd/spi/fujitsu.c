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
		.page_size = 32 * 1024, /* Whole chip can be written at once */
		.flags = SPI_NOR_NO_ERASE,
		.addr_width = 2,
	},
};

const struct spi_nor_manufacturer spi_nor_fujitsu = {
	.parts = fujitsu_nor_parts,
	.nparts = ARRAY_SIZE(fujitsu_nor_parts),
};
