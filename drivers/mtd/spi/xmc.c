// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info xmc_nor_parts[] = {
	{ INFO("XM25QH64A", 0x207017, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("XM25QH64C", 0x204017, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("XM25QH128A", 0x207018, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("XM25QH128C", 0x204018, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("XM25QU128C", 0x204118, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("XM25QH256C", 0x204019, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("XM25QU256C", 0x204119, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("XM25QH512D", 0x204020, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("XM25QU512C", 0x204120, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("XM25QH01D", 0x204021, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
};

const struct spi_nor_manufacturer spi_nor_xmc = {
	.parts = xmc_nor_parts,
	.nparts = ARRAY_SIZE(xmc_nor_parts),
};
