// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info puya_nor_parts[] = {
	{ INFO
	 ("p25q05h", 0x856010, 0, 64 * 1024, 1,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q10h", 0x856011, 0, 64 * 1024, 2,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q20h", 0x856012, 0, 64 * 1024, 4,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q40h", 0x856013, 0, 64 * 1024, 8,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q80h", 0x856014, 0, 64 * 1024, 16,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q16h", 0x856015, 0, 64 * 1024, 32,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q32h", 0x856016, 0, 64 * 1024, 64,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q64h", 0x856017, 0, 64 * 1024, 128,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("p25q128h", 0x856018, 0, 64 * 1024, 256,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO
	 ("py25f512hb", 0x85231a, 0, 64 * 1024, 1024,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO
	 ("py25f01ghb", 0x85231b, 0, 64 * 1024, 2048,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO
	 ("py25f512lc", 0x85631a, 0, 64 * 1024, 1024,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO
	 ("py25f01glc", 0x85631b, 0, 64 * 1024, 2048,
	  SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
};

const struct spi_nor_manufacturer spi_nor_puya = {
	.parts = puya_nor_parts,
	.nparts = ARRAY_SIZE(puya_nor_parts),
};
