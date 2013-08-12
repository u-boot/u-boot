/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 *
 * Author: Mingkai Hu (Mingkai.hu@freescale.com)
 * Based on stmicro.c by Wolfgang Denk (wd@denx.de),
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com),
 * and  Jason McMullan (mcmullan@netapp.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct spansion_spi_flash_params {
	u16 idcode1;
	u16 idcode2;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

static const struct spansion_spi_flash_params spansion_spi_flash_table[] = {
	{
		.idcode1 = 0x0213,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "S25FL008A",
	},
	{
		.idcode1 = 0x0214,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "S25FL016A",
	},
	{
		.idcode1 = 0x0215,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032A",
	},
	{
		.idcode1 = 0x0216,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "S25FL064A",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x0301,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128P_64K",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x0300,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "S25FL128P_256K",
	},
	{
		.idcode1 = 0x0215,
		.idcode2 = 0x4d00,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032P",
	},
	{
		.idcode1 = 0x0216,
		.idcode2 = 0x4d00,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "S25FL064P",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL129P_64K/S25FL128S_64K",
	},
	{
		.idcode1 = 0x0219,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 512,
		.name = "S25FL256S_64K",
	},
	{
		.idcode1 = 0x0220,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 1024,
		.name = "S25FL512S_64K",
	},
};

struct spi_flash *spi_flash_probe_spansion(struct spi_slave *spi, u8 *idcode)
{
	const struct spansion_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;
	unsigned short jedec, ext_jedec;

	jedec = idcode[1] << 8 | idcode[2];
	ext_jedec = idcode[3] << 8 | idcode[4];

	for (i = 0; i < ARRAY_SIZE(spansion_spi_flash_table); i++) {
		params = &spansion_spi_flash_table[i];
		if (params->idcode1 == jedec) {
			if (params->idcode2 == ext_jedec)
				break;
		}
	}

	if (i == ARRAY_SIZE(spansion_spi_flash_table)) {
		debug("SF: Unsupported SPANSION ID %04x %04x\n",
		      jedec, ext_jedec);
		return NULL;
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->page_size = 256;
	flash->sector_size = 256 * params->pages_per_sector;
	flash->size = flash->sector_size * params->nr_sectors;

	return flash;
}
