/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct winbond_spi_flash_params {
	uint16_t	id;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= 0x2014,
		.nr_blocks		= 16,
		.name			= "W25P80",
	},
	{
		.id			= 0x2015,
		.nr_blocks		= 32,
		.name			= "W25P16",
	},
	{
		.id			= 0x2016,
		.nr_blocks		= 64,
		.name			= "W25P32",
	},
	{
		.id			= 0x3013,
		.nr_blocks		= 8,
		.name			= "W25X40",
	},
	{
		.id			= 0x3015,
		.nr_blocks		= 32,
		.name			= "W25X16",
	},
	{
		.id			= 0x3016,
		.nr_blocks		= 64,
		.name			= "W25X32",
	},
	{
		.id			= 0x3017,
		.nr_blocks		= 128,
		.name			= "W25X64",
	},
	{
		.id			= 0x4014,
		.nr_blocks		= 16,
		.name			= "W25Q80BL/W25Q80BV",
	},
	{
		.id			= 0x4015,
		.nr_blocks		= 32,
		.name			= "W25Q16CL/W25Q16DV",
	},
	{
		.id			= 0x4016,
		.nr_blocks		= 64,
		.name			= "W25Q32BV/W25Q32FV_SPI",
	},
	{
		.id			= 0x4017,
		.nr_blocks		= 128,
		.name			= "W25Q64CV/W25Q64FV_SPI",
	},
	{
		.id			= 0x4018,
		.nr_blocks		= 256,
		.name			= "W25Q128BV/W25Q128FV_SPI",
	},
	{
		.id			= 0x4019,
		.nr_blocks		= 512,
		.name			= "W25Q256",
	},
	{
		.id			= 0x5014,
		.nr_blocks		= 16,
		.name			= "W25Q80BW",
	},
	{
		.id			= 0x6015,
		.nr_blocks		= 32,
		.name			= "W25Q16DW",
	},
	{
		.id			= 0x6016,
		.nr_blocks		= 64,
		.name			= "W25Q32DW/W25Q32FV_QPI",
	},
	{
		.id			= 0x6017,
		.nr_blocks		= 128,
		.name			= "W25Q64DW/W25Q64FV_QPI",
	},
	{
		.id			= 0x6018,
		.nr_blocks		= 256,
		.name			= "W25Q128FW/W25Q128FV_QPI",
	},
};

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode)
{
	const struct winbond_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		debug("SF: Unsupported Winbond ID %02x%02x\n",
		      idcode[1], idcode[2]);
		return NULL;
	}

	flash = spi_flash_alloc_base(spi, params->name);
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->page_size = 256;
	flash->sector_size = (idcode[1] == 0x20) ? 65536 : 4096;
	flash->size = 4096 * 16 * params->nr_blocks;

	return flash;
}
