/*
 * Copyright 2009(C) Marvell International Ltd. and its affiliates
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Based on drivers/mtd/spi/stmicro.c
 *
 * Copyright 2008, Network Appliance Inc.
 * Jason McMullan <mcmullan@netapp.com>
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct macronix_spi_flash_params {
	u16 idcode;
	u16 nr_blocks;
	const char *name;
};

static const struct macronix_spi_flash_params macronix_spi_flash_table[] = {
	{
		.idcode = 0x2013,
		.nr_blocks = 8,
		.name = "MX25L4005",
	},
	{
		.idcode = 0x2014,
		.nr_blocks = 16,
		.name = "MX25L8005",
	},
	{
		.idcode = 0x2015,
		.nr_blocks = 32,
		.name = "MX25L1605D",
	},
	{
		.idcode = 0x2016,
		.nr_blocks = 64,
		.name = "MX25L3205D",
	},
	{
		.idcode = 0x2017,
		.nr_blocks = 128,
		.name = "MX25L6405D",
	},
	{
		.idcode = 0x2018,
		.nr_blocks = 256,
		.name = "MX25L12805D",
	},
	{
		.idcode = 0x2618,
		.nr_blocks = 256,
		.name = "MX25L12855E",
	},
};

struct spi_flash *spi_flash_probe_macronix(struct spi_slave *spi, u8 *idcode)
{
	const struct macronix_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;
	u16 id = idcode[2] | idcode[1] << 8;

	for (i = 0; i < ARRAY_SIZE(macronix_spi_flash_table); i++) {
		params = &macronix_spi_flash_table[i];
		if (params->idcode == id)
			break;
	}

	if (i == ARRAY_SIZE(macronix_spi_flash_table)) {
		debug("SF: Unsupported Macronix ID %04x\n", id);
		return NULL;
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->page_size = 256;
	flash->sector_size = 256 * 16 * 16;
	flash->size = flash->sector_size * params->nr_blocks;

	/* Clear BP# bits for read-only flash */
	spi_flash_cmd_write_status(flash, 0);

	return flash;
}
