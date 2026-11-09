// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info eon_nor_parts[] = {
	{ INFO("en25q80b",   0x1c3014, 0, 64 * 1024,   16, SECT_4K) },
	{ INFO("en25q32b",   0x1c3016, 0, 64 * 1024,   64, 0) },
	{ INFO("en25q64",    0x1c3017, 0, 64 * 1024,  128, SECT_4K) },
	{ INFO("en25q128b",  0x1c3018, 0, 64 * 1024,  256, 0) },
	{ INFO("en25qh128",  0x1c7018, 0, 64 * 1024,  256, 0) },
	{ INFO("en25s64",    0x1c3817, 0, 64 * 1024,  128, SECT_4K) },
};

const struct spi_nor_manufacturer spi_nor_eon = {
	.parts = eon_nor_parts,
	.nparts = ARRAY_SIZE(eon_nor_parts),
};
