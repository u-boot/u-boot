/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 *
 * Author: Mingkai Hu (Mingkai.hu@freescale.com)
 * Based on stmicro.c by Wolfgang Denk (wd@denx.de),
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com),
 * and  Jason McMullan (mcmullan@netapp.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* S25FLxx-specific commands */
#define CMD_S25FLXX_READ	0x03	/* Read Data Bytes */
#define CMD_S25FLXX_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_S25FLXX_READID	0x90	/* Read Manufacture ID and Device ID */
#define CMD_S25FLXX_WREN	0x06	/* Write Enable */
#define CMD_S25FLXX_WRDI	0x04	/* Write Disable */
#define CMD_S25FLXX_RDSR	0x05	/* Read Status Register */
#define CMD_S25FLXX_WRSR	0x01	/* Write Status Register */
#define CMD_S25FLXX_PP		0x02	/* Page Program */
#define CMD_S25FLXX_SE		0xd8	/* Sector Erase */
#define CMD_S25FLXX_BE		0xc7	/* Bulk Erase */
#define CMD_S25FLXX_DP		0xb9	/* Deep Power-down */
#define CMD_S25FLXX_RES		0xab	/* Release from DP, and Read Signature */

#define SPSN_ID_S25FL008A	0x0213
#define SPSN_ID_S25FL016A	0x0214
#define SPSN_ID_S25FL032A	0x0215
#define SPSN_ID_S25FL064A	0x0216
#define SPSN_ID_S25FL128P	0x2018
#define SPSN_EXT_ID_S25FL128P_256KB	0x0300
#define SPSN_EXT_ID_S25FL128P_64KB	0x0301
#define SPSN_EXT_ID_S25FL032P		0x4d00

struct spansion_spi_flash_params {
	u16 idcode1;
	u16 idcode2;
	u16 page_size;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

struct spansion_spi_flash {
	struct spi_flash flash;
	const struct spansion_spi_flash_params *params;
};

static inline struct spansion_spi_flash *to_spansion_spi_flash(struct spi_flash
							     *flash)
{
	return container_of(flash, struct spansion_spi_flash, flash);
}

static const struct spansion_spi_flash_params spansion_spi_flash_table[] = {
	{
		.idcode1 = SPSN_ID_S25FL008A,
		.idcode2 = 0,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "S25FL008A",
	},
	{
		.idcode1 = SPSN_ID_S25FL016A,
		.idcode2 = 0,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "S25FL016A",
	},
	{
		.idcode1 = SPSN_ID_S25FL032A,
		.idcode2 = 0,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032A",
	},
	{
		.idcode1 = SPSN_ID_S25FL064A,
		.idcode2 = 0,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "S25FL064A",
	},
	{
		.idcode1 = SPSN_ID_S25FL128P,
		.idcode2 = SPSN_EXT_ID_S25FL128P_64KB,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128P_64K",
	},
	{
		.idcode1 = SPSN_ID_S25FL128P,
		.idcode2 = SPSN_EXT_ID_S25FL128P_256KB,
		.page_size = 256,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "S25FL128P_256K",
	},
	{
		.idcode1 = SPSN_ID_S25FL032A,
		.idcode2 = SPSN_EXT_ID_S25FL032P,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032P",
	},
};

static int spansion_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_S25FLXX_SE, offset, len);
}

struct spi_flash *spi_flash_probe_spansion(struct spi_slave *spi, u8 *idcode)
{
	const struct spansion_spi_flash_params *params;
	struct spansion_spi_flash *spsn;
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
		debug("SF: Unsupported SPANSION ID %04x %04x\n", jedec, ext_jedec);
		return NULL;
	}

	spsn = malloc(sizeof(struct spansion_spi_flash));
	if (!spsn) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	spsn->params = params;
	spsn->flash.spi = spi;
	spsn->flash.name = params->name;

	spsn->flash.write = spi_flash_cmd_write_multi;
	spsn->flash.erase = spansion_erase;
	spsn->flash.read = spi_flash_cmd_read_fast;
	spsn->flash.page_size = params->page_size;
	spsn->flash.sector_size = params->page_size * params->pages_per_sector;
	spsn->flash.size = spsn->flash.sector_size * params->nr_sectors;

	return &spsn->flash;
}
