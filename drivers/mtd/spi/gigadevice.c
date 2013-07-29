/*
 * Gigadevice SPI flash driver
 * Copyright 2013, Samsung Electronics Co., Ltd.
 * Author: Banajit Goswami <banajit.g@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct gigadevice_spi_flash_params {
	uint16_t	id;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct gigadevice_spi_flash_params gigadevice_spi_flash_table[] = {
	{
		.id			= 0x6016,
		.nr_blocks		= 64,
		.name			= "GD25LQ",
	},
	{
		.id			= 0x4017,
		.nr_blocks		= 128,
		.name			= "GD25Q64B",
	},
};

struct spi_flash *spi_flash_probe_gigadevice(struct spi_slave *spi, u8 *idcode)
{
	const struct gigadevice_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(gigadevice_spi_flash_table); i++) {
		params = &gigadevice_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(gigadevice_spi_flash_table)) {
		debug("SF: Unsupported Gigadevice ID %02x%02x\n",
		      idcode[1], idcode[2]);
		return NULL;
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}
	/* page_size */
	flash->page_size = 256;
	/* sector_size = page_size * pages_per_sector */
	flash->sector_size = flash->page_size * 16;
	/* size = sector_size * sector_per_block * number of blocks */
	flash->size = flash->sector_size * 16 * params->nr_blocks;

	return flash;
}
