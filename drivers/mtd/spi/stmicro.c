/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/* M25Pxx-specific commands */
#define CMD_M25PXX_RES	0xab	/* Release from DP, and Read Signature */

struct stmicro_spi_flash_params {
	u16 id;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

static const struct stmicro_spi_flash_params stmicro_spi_flash_table[] = {
	{
		.id = 0x2011,
		.pages_per_sector = 128,
		.nr_sectors = 4,
		.name = "M25P10",
	},
	{
		.id = 0x2015,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "M25P16",
	},
	{
		.id = 0x2012,
		.pages_per_sector = 256,
		.nr_sectors = 4,
		.name = "M25P20",
	},
	{
		.id = 0x2016,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "M25P32",
	},
	{
		.id = 0x2013,
		.pages_per_sector = 256,
		.nr_sectors = 8,
		.name = "M25P40",
	},
	{
		.id = 0x2017,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "M25P64",
	},
	{
		.id = 0x2014,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "M25P80",
	},
	{
		.id = 0x2018,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "M25P128",
	},
	{
		.id = 0xba16,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "N25Q32",
	},
	{
		.id = 0xbb16,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "N25Q32A",
	},
	{
		.id = 0xba17,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "N25Q064",
	},
	{
		.id = 0xbb17,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "N25Q64A",
	},
	{
		.id = 0xba18,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "N25Q128",
	},
	{
		.id = 0xbb18,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "N25Q128A",
	},
	{
		.id = 0xba19,
		.pages_per_sector = 256,
		.nr_sectors = 512,
		.name = "N25Q256",
	},
	{
		.id = 0xbb19,
		.pages_per_sector = 256,
		.nr_sectors = 512,
		.name = "N25Q256A",
	},
	{
		.id = 0xba20,
		.pages_per_sector = 256,
		.nr_sectors = 1024,
		.name = "N25Q512",
	},
	{
		.id = 0xbb20,
		.pages_per_sector = 256,
		.nr_sectors = 1024,
		.name = "N25Q512A",
	},
	{
		.id = 0xba21,
		.pages_per_sector = 256,
		.nr_sectors = 2048,
		.name = "N25Q1024",
	},
	{
		.id = 0xbb21,
		.pages_per_sector = 256,
		.nr_sectors = 2048,
		.name = "N25Q1024A",
	},
};

struct spi_flash *spi_flash_probe_stmicro(struct spi_slave *spi, u8 *idcode)
{
	const struct stmicro_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;
	u16 id;

	if (idcode[0] == 0xff) {
		i = spi_flash_cmd(spi, CMD_M25PXX_RES,
				  idcode, 4);
		if (i)
			return NULL;
		if ((idcode[3] & 0xf0) == 0x10) {
			idcode[0] = 0x20;
			idcode[1] = 0x20;
			idcode[2] = idcode[3] + 1;
		} else {
			return NULL;
		}
	}

	id = ((idcode[1] << 8) | idcode[2]);

	for (i = 0; i < ARRAY_SIZE(stmicro_spi_flash_table); i++) {
		params = &stmicro_spi_flash_table[i];
		if (params->id == id)
			break;
	}

	if (i == ARRAY_SIZE(stmicro_spi_flash_table)) {
		debug("SF: Unsupported STMicro ID %04x\n", id);
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

	/* for >= 512MiB flashes, use flag status instead of read_status */
	if (flash->size >= 0x4000000)
		flash->poll_cmd = CMD_FLAG_STATUS;

	return flash;
}
