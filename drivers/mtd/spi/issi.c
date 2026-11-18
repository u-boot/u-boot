// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info issi_nor_parts[] = {
	{ INFO("is25lq040b", 0x9d4013, 0, 64 * 1024,   8,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp008", 0x9d6014, 0, 64 * 1024,  16, SPI_NOR_QUAD_READ) },
	{ INFO("is25lp016", 0x9d6015, 0, 64 * 1024,  32, SPI_NOR_QUAD_READ) },
	{ INFO("is25lp032",	0x9d6016, 0, 64 * 1024,  64, 0) },
	{ INFO("is25lp064",	0x9d6017, 0, 64 * 1024, 128, 0) },
	{ INFO("is25lp128",  0x9d6018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ) },
	{ INFO("is25lp256",  0x9d6019, 0, 64 * 1024, 512,
			SECT_4K | SPI_NOR_DUAL_READ) },
	{ INFO("is25lp512",  0x9d601a, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp512mj", 0x9d6020, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp01g",  0x9d601b, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp008", 0x9d7014, 0, 64 * 1024,  16, SPI_NOR_QUAD_READ) },
	{ INFO("is25wp016", 0x9d7015, 0, 64 * 1024,  32, SPI_NOR_QUAD_READ) },
	{ INFO("is25wp032",  0x9d7016, 0, 64 * 1024,  64,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp064",  0x9d7017, 0, 64 * 1024, 128,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp128",  0x9d7018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp256",  0x9d7019, 0, 64 * 1024, 512,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_4B_OPCODES) },
	{ INFO("is25wp512",  0x9d701a, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp512mj", 0x9d7020, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp01g",  0x9d701b, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wp01gg",  0x9d7021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25wx128",  0x9d5b18, 0, 64 * 1024, 256,
			SECT_4K | USE_FSR | SPI_NOR_OCTAL_READ) },
	{ INFO("is25wx256",  0x9d5b19, 0, 128 * 1024, 256,
			SECT_4K | USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("is25lx128",  0x9d5a18, 0, 64 * 1024, 256,
			SECT_4K | USE_FSR | SPI_NOR_OCTAL_READ) },
	{ INFO("is25lx512",  0x9d5a1a, 0, 64 * 1024, 1024,
			SECT_4K | USE_FSR | SPI_NOR_4B_OPCODES | SPI_NOR_HAS_TB) },
	{ INFO("is25lp01gg",  0x9d6021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25lp010e",  0x9d4011, 0, 64 * 1024, 2,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp020e",  0x9d4012, 0, 64 * 1024, 4,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp040e",  0x9d4013, 0, 64 * 1024, 8,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("is25lp01gj",  0x9d6021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25lp02gg",  0x9d6022, 0, 64 * 1024, 4096,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25lp02gj",  0x9d6022, 0, 64 * 1024, 4096,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25wp01gg",  0x9d7021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25wp01gj",  0x9d7021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25wj128f",  0x9d7118, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25wp02gg",  0x9d7022, 0, 64 * 1024, 4096,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
	{ INFO("is25wp02gj",  0x9d7022, 0, 64 * 1024, 4096,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_HAS_TB) },
};

const struct spi_nor_manufacturer spi_nor_issi = {
	.parts = issi_nor_parts,
	.nparts = ARRAY_SIZE(issi_nor_parts),
};
