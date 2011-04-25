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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* MX25xx-specific commands */
#define CMD_MX25XX_WREN		0x06	/* Write Enable */
#define CMD_MX25XX_WRDI		0x04	/* Write Disable */
#define CMD_MX25XX_RDSR		0x05	/* Read Status Register */
#define CMD_MX25XX_WRSR		0x01	/* Write Status Register */
#define CMD_MX25XX_READ		0x03	/* Read Data Bytes */
#define CMD_MX25XX_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_MX25XX_PP		0x02	/* Page Program */
#define CMD_MX25XX_SE		0x20	/* Sector Erase */
#define CMD_MX25XX_BE		0xD8	/* Block Erase */
#define CMD_MX25XX_CE		0xc7	/* Chip Erase */
#define CMD_MX25XX_DP		0xb9	/* Deep Power-down */
#define CMD_MX25XX_RES		0xab	/* Release from DP, and Read Signature */

struct macronix_spi_flash_params {
	u16 idcode;
	u16 page_size;
	u16 pages_per_sector;
	u16 sectors_per_block;
	u16 nr_blocks;
	const char *name;
};

struct macronix_spi_flash {
	struct spi_flash flash;
	const struct macronix_spi_flash_params *params;
};

static inline struct macronix_spi_flash *to_macronix_spi_flash(struct spi_flash
							       *flash)
{
	return container_of(flash, struct macronix_spi_flash, flash);
}

static const struct macronix_spi_flash_params macronix_spi_flash_table[] = {
	{
		.idcode = 0x2015,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 32,
		.name = "MX25L1605D",
	},
	{
		.idcode = 0x2016,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 64,
		.name = "MX25L3205D",
	},
	{
		.idcode = 0x2017,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 128,
		.name = "MX25L6405D",
	},
	{
		.idcode = 0x2018,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 256,
		.name = "MX25L12805D",
	},
	{
		.idcode = 0x2618,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 256,
		.name = "MX25L12855E",
	},
};

static int macronix_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_MX25XX_BE, offset, len);
}

struct spi_flash *spi_flash_probe_macronix(struct spi_slave *spi, u8 *idcode)
{
	const struct macronix_spi_flash_params *params;
	struct macronix_spi_flash *mcx;
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

	mcx = malloc(sizeof(*mcx));
	if (!mcx) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	mcx->params = params;
	mcx->flash.spi = spi;
	mcx->flash.name = params->name;

	mcx->flash.write = spi_flash_cmd_write_multi;
	mcx->flash.erase = macronix_erase;
	mcx->flash.read = spi_flash_cmd_read_fast;
	mcx->flash.page_size = params->page_size;
	mcx->flash.sector_size = params->page_size * params->pages_per_sector
		* params->sectors_per_block;
	mcx->flash.size = mcx->flash.sector_size * params->nr_blocks;

	return &mcx->flash;
}
