// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info atmel_nor_parts[] = {
	{ INFO("at26df321",	0x1f4700, 0, 64 * 1024, 64, SECT_4K) },
	{ INFO("at25df321a",	0x1f4701, 0, 64 * 1024, 64, SECT_4K) },
	{ INFO("at45db011d",	0x1f2200, 0, 64 * 1024,   4, SECT_4K) },
	{ INFO("at45db021d",	0x1f2300, 0, 64 * 1024,   8, SECT_4K) },
	{ INFO("at45db041d",	0x1f2400, 0, 64 * 1024,   8, SECT_4K) },
	{ INFO("at45db081d",	0x1f2500, 0, 64 * 1024,  16, SECT_4K) },
	{ INFO("at45db161d",	0x1f2600, 0, 64 * 1024,  32, SECT_4K) },
	{ INFO("at45db321d",	0x1f2700, 0, 64 * 1024,  64, SECT_4K) },
	{ INFO("at45db641d",	0x1f2800, 0, 64 * 1024, 128, SECT_4K) },
	{ INFO("at25sl321",	0x1f4216, 0, 64 * 1024,  64, SECT_4K) },
	{ INFO("at26df081a",	0x1f4501, 0, 64 * 1024,  16, SECT_4K) },
};

const struct spi_nor_manufacturer spi_nor_atmel = {
	.parts = atmel_nor_parts,
	.nparts = ARRAY_SIZE(atmel_nor_parts),
};
