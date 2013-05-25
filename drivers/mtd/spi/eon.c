/*
 * (C) Copyright 2010, ucRobotics Inc.
 * Author: Chong Huang <chuang@ucrobotics.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct eon_spi_flash_params {
	u8 idcode1;
	u16 nr_sectors;
	const char *name;
};

static const struct eon_spi_flash_params eon_spi_flash_table[] = {
	{
		.idcode1 = 0x16,
		.nr_sectors = 1024,
		.name = "EN25Q32B",
	},
	{
		.idcode1 = 0x18,
		.nr_sectors = 4096,
		.name = "EN25Q128",
	},
};

struct spi_flash *spi_flash_probe_eon(struct spi_slave *spi, u8 *idcode)
{
	const struct eon_spi_flash_params *params;
	struct spi_flash *flash;
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

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->page_size = 256;
	flash->sector_size = 256 * 16 * 16;
	flash->size = 256 * 16 * params->nr_sectors;

	return flash;
}
