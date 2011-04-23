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

static int eon_write(struct spi_flash *flash,
		     u32 offset, size_t len, const void *buf)
{
	struct eon_spi_flash *eon = to_eon_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	page_size = eon->params->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = 0;
	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = CMD_EN25Q128_PP;
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		debug
		    ("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
		     buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd_write_enable(flash);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
					  buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: EON Page Program failed\n");
			break;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			break;

		page_addr++;
		byte_addr = 0;
	}

	debug("SF: EON: Successfully programmed %u bytes @ 0x%x\n",
	      len, offset);

	spi_release_bus(flash->spi);
	return ret;
}

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

	eon->flash.write = eon_write;
	eon->flash.erase = eon_erase;
	eon->flash.read = spi_flash_cmd_read_fast;
	eon->flash.sector_size = params->page_size * params->pages_per_sector
	    * params->sectors_per_block;
	eon->flash.size = params->page_size * params->pages_per_sector
	    * params->nr_sectors;

	return &eon->flash;
}
