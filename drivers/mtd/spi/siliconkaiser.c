// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info siliconkaiser_nor_parts[] = {
	{ INFO("sk25lp128", 0x257018, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
};

const struct spi_nor_manufacturer spi_nor_siliconkaiser = {
	.parts = siliconkaiser_nor_parts,
	.nparts = ARRAY_SIZE(siliconkaiser_nor_parts),
};
