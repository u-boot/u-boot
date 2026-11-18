// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info sst_nor_parts[] = {
	{ INFO("sst25vf040b", 0xbf258d, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
	{ INFO("sst25vf080b", 0xbf258e, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
	{ INFO("sst25vf016b", 0xbf2541, 0, 64 * 1024, 32, SECT_4K | SST_WRITE) },
	{ INFO("sst25vf032b", 0xbf254a, 0, 64 * 1024, 64, SECT_4K | SST_WRITE) },
	{ INFO("sst25vf064c", 0xbf254b, 0, 64 * 1024, 128, SECT_4K) },
	{ INFO("sst25wf512",  0xbf2501, 0, 64 * 1024,  1, SECT_4K | SST_WRITE) },
	{ INFO("sst25wf010",  0xbf2502, 0, 64 * 1024,  2, SECT_4K | SST_WRITE) },
	{ INFO("sst25wf020",  0xbf2503, 0, 64 * 1024,  4, SECT_4K | SST_WRITE) },
	{ INFO("sst25wf020a", 0x621612, 0, 64 * 1024,  4, SECT_4K) },
	{ INFO("sst25wf040b", 0x621613, 0, 64 * 1024,  8, SECT_4K) },
	{ INFO("sst25wf040",  0xbf2504, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
	{ INFO("sst25wf080",  0xbf2505, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
	{ INFO("sst26vf064b", 0xbf2643, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_HAS_SST26LOCK | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("sst26wf016b", 0xbf2641, 0, 64 * 1024,  32, SECT_4K) },
	{ INFO("sst26wf016",  0xbf2651, 0, 64 * 1024,  32, SECT_4K | SPI_NOR_HAS_SST26LOCK) },
	{ INFO("sst26wf032",  0xbf2622, 0, 64 * 1024,  64, SECT_4K | SPI_NOR_HAS_SST26LOCK) },
	{ INFO("sst26wf064",  0xbf2643, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_HAS_SST26LOCK) },
};

const struct spi_nor_manufacturer spi_nor_sst = {
	.parts = sst_nor_parts,
	.nparts = ARRAY_SIZE(sst_nor_parts),
};
