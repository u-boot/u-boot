// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info dosilicon_nor_parts[] = {
	{ INFO("ds25m4cb", 0xe5401a, 0, 64 * 1024, 1024,
		    SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("ds25m4dn", 0xe5401b, 0, 64 * 1024, 2048,
		    SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("ds25q4cb", 0xe5301a, 0, 64 * 1024, 1024,
		    SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("ds25q4dn", 0xe5301b, 0, 64 * 1024, 2048,
		    SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("ds25q4bb", 0xe53019, 0, 64 * 1024, 512,
		    SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
};

const struct spi_nor_manufacturer spi_nor_dosilicon = {
	.parts = dosilicon_nor_parts,
	.nparts = ARRAY_SIZE(dosilicon_nor_parts),
};
