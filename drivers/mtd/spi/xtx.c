// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info xtx_nor_parts[] = {
	/* adding these 3V QSPI flash parts */
	{ INFO("xt25f08", 0x0b4014, 0, 64 * 1024, 16,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25f16", 0x0b4015, 0, 64 * 1024, 32,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25f32", 0x0b4016, 0, 64 * 1024, 64,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25f64", 0x0b4017, 0, 64 * 1024, 128,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25f128", 0x0b4018, 0, 64 * 1024, 256,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25f256", 0x0b4019, 0, 64 * 1024, 512,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	/* adding these 1.8V QSPI flash parts */
	{ INFO("xt25q08", 0x0b6014, 0, 64 * 1024, 16,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25q16", 0x0b6015, 0, 64 * 1024, 32,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25q32", 0x0b6016, 0, 64 * 1024, 64,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25q64", 0x0b6017, 0, 64 * 1024, 128,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25q128", 0x0b6018, 0, 64 * 1024, 256,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("xt25q256", 0x0b6019, 0, 64 * 1024, 512,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("xt25q512", 0x0b601A, 0, 64 * 1024, 1024,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("xt25q01g", 0x0b601B, 0, 64 * 1024, 2048,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("xt55q02g", 0x0b601C, 0, 64 * 1024, 4096,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	/* adding these wide voltage QSPI flash parts */
	{ INFO("xt25w512", 0x0b651A, 0, 64 * 1024, 1024,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("xt25w01g", 0x0b651B, 0, 64 * 1024, 2048,
	       SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
};

const struct spi_nor_manufacturer spi_nor_xtx = {
	.parts = xtx_nor_parts,
	.nparts = ARRAY_SIZE(xtx_nor_parts),
};
