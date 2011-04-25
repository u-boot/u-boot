/*
 * (C) Copyright 2010, ucRobotics Inc.
 * Author: Chong Huang <chuang@ucrobotics.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* EN25Q128-specific commands */
#define CMD_EN25Q128_WREN	0x06    /* Write Enable */
#define CMD_EN25Q128_WRDI	0x04    /* Write Disable */
#define CMD_EN25Q128_RDSR	0x05    /* Read Status Register */
#define CMD_EN25Q128_WRSR	0x01    /* Write Status Register */
#define CMD_EN25Q128_READ	0x03    /* Read Data Bytes */
#define CMD_EN25Q128_FAST_READ	0x0b    /* Read Data Bytes at Higher Speed */
#define CMD_EN25Q128_PP		0x02    /* Page Program */
#define CMD_EN25Q128_SE		0x20    /* Sector Erase */
#define CMD_EN25Q128_BE		0xd8    /* Block Erase */
#define CMD_EN25Q128_DP		0xb9    /* Deep Power-down */
#define CMD_EN25Q128_RES	0xab    /* Release from DP, and Read Signature */

#define EON_ID_EN25Q128		0x18

struct eon_spi_flash_params {
	u8 idcode1;
	u16 page_size;
	u16 pages_per_sector;
	u16 sectors_per_block;
	u16 nr_sectors;
	const char *name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct eon_spi_flash {
	struct spi_flash flash;
	const struct eon_spi_flash_params *params;
};

static inline struct eon_spi_flash *to_eon_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct eon_spi_flash, flash);
}

static const struct eon_spi_flash_params eon_spi_flash_table[] = {
	{
		.idcode1 = EON_ID_EN25Q128,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_sectors = 4096,
		.name = "EN25Q128",
	},
};

static int eon_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_EN25Q128_BE, offset, len);
}

struct spi_flash *spi_flash_probe_eon(struct spi_slave *spi, u8 *idcode)
{
	const struct eon_spi_flash_params *params;
	struct eon_spi_flash *eon;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(eon_spi_flash_table); ++i) {
		params = &eon_spi_flash_table[i];
		if (params->idcode1 == idcode[2])
			break;
	}

	if (i == ARRAY_SIZE(eon_spi_flash_table)) {
		debug("SF: Unsupported EON ID %02x\n", idcode[1]);
		return NULL;
	}

	eon = malloc(sizeof(*eon));
	if (!eon) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	eon->params = params;
	eon->flash.spi = spi;
	eon->flash.name = params->name;

	eon->flash.write = spi_flash_cmd_write_multi;
	eon->flash.erase = eon_erase;
	eon->flash.read = spi_flash_cmd_read_fast;
	eon->flash.page_size = params->page_size;
	eon->flash.sector_size = params->page_size * params->pages_per_sector
	    * params->sectors_per_block;
	eon->flash.size = params->page_size * params->pages_per_sector
	    * params->nr_sectors;

	return &eon->flash;
}
