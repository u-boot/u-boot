/*
 * Driver for SST serial flashes
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright 2008, Network Appliance Inc.
 * Jason McMullan <mcmullan@netapp.com>
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

#define CMD_SST_BP		0x02	/* Byte Program */
#define CMD_SST_AAI_WP		0xAD	/* Auto Address Incr Word Program */

#define SST_SR_WIP		(1 << 0)	/* Write-in-Progress */
#define SST_SR_WEL		(1 << 1)	/* Write enable */
#define SST_SR_BP0		(1 << 2)	/* Block Protection 0 */
#define SST_SR_BP1		(1 << 3)	/* Block Protection 1 */
#define SST_SR_BP2		(1 << 4)	/* Block Protection 2 */
#define SST_SR_AAI		(1 << 6)	/* Addressing mode */
#define SST_SR_BPL		(1 << 7)	/* BP bits lock */

#define SST_FEAT_WP		(1 << 0)	/* Supports AAI word program */
#define SST_FEAT_MBP		(1 << 1)	/* Supports multibyte program */

struct sst_spi_flash_params {
	u8 idcode1;
	u8 flags;
	u16 nr_sectors;
	const char *name;
};

struct sst_spi_flash {
	struct spi_flash flash;
	const struct sst_spi_flash_params *params;
};

static const struct sst_spi_flash_params sst_spi_flash_table[] = {
	{
		.idcode1 = 0x8d,
		.flags = SST_FEAT_WP,
		.nr_sectors = 128,
		.name = "SST25VF040B",
	},
	{
		.idcode1 = 0x8e,
		.flags = SST_FEAT_WP,
		.nr_sectors = 256,
		.name = "SST25VF080B",
	},
	{
		.idcode1 = 0x41,
		.flags = SST_FEAT_WP,
		.nr_sectors = 512,
		.name = "SST25VF016B",
	},
	{
		.idcode1 = 0x4a,
		.flags = SST_FEAT_WP,
		.nr_sectors = 1024,
		.name = "SST25VF032B",
	},
	{
		.idcode1 = 0x4b,
		.flags = SST_FEAT_MBP,
		.nr_sectors = 2048,
		.name = "SST25VF064C",
	},
	{
		.idcode1 = 0x01,
		.flags = SST_FEAT_WP,
		.nr_sectors = 16,
		.name = "SST25WF512",
	},
	{
		.idcode1 = 0x02,
		.flags = SST_FEAT_WP,
		.nr_sectors = 32,
		.name = "SST25WF010",
	},
	{
		.idcode1 = 0x03,
		.flags = SST_FEAT_WP,
		.nr_sectors = 64,
		.name = "SST25WF020",
	},
	{
		.idcode1 = 0x04,
		.flags = SST_FEAT_WP,
		.nr_sectors = 128,
		.name = "SST25WF040",
	},
	{
		.idcode1 = 0x05,
		.flags = SST_FEAT_WP,
		.nr_sectors = 256,
		.name = "SST25WF080",
	},
};

static int
sst_byte_write(struct spi_flash *flash, u32 offset, const void *buf)
{
	int ret;
	u8 cmd[4] = {
		CMD_SST_BP,
		offset >> 16,
		offset >> 8,
		offset,
	};

	debug("BP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
	      spi_w8r8(flash->spi, CMD_READ_STATUS), buf, cmd[0], offset);

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		return ret;

	ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), buf, 1);
	if (ret)
		return ret;

	return spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
}

static int
sst_write_wp(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
{
	size_t actual, cmd_len;
	int ret;
	u8 cmd[4];

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	/* If the data is not word aligned, write out leading single byte */
	actual = offset % 2;
	if (actual) {
		ret = sst_byte_write(flash, offset, buf);
		if (ret)
			goto done;
	}
	offset += actual;

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		goto done;

	cmd_len = 4;
	cmd[0] = CMD_SST_AAI_WP;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset;

	for (; actual < len - 1; actual += 2) {
		debug("WP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
		      spi_w8r8(flash->spi, CMD_READ_STATUS), buf + actual,
		      cmd[0], offset);

		ret = spi_flash_cmd_write(flash->spi, cmd, cmd_len,
					buf + actual, 2);
		if (ret) {
			debug("SF: sst word program failed\n");
			break;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			break;

		cmd_len = 1;
		offset += 2;
	}

	if (!ret)
		ret = spi_flash_cmd_write_disable(flash);

	/* If there is a single trailing byte, write it out */
	if (!ret && actual != len)
		ret = sst_byte_write(flash, offset, buf + actual);

 done:
	debug("SF: sst: program %s %zu bytes @ 0x%zx\n",
	      ret ? "failure" : "success", len, offset - actual);

	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *
spi_flash_probe_sst(struct spi_slave *spi, u8 *idcode)
{
	const struct sst_spi_flash_params *params;
	struct sst_spi_flash *stm;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(sst_spi_flash_table); ++i) {
		params = &sst_spi_flash_table[i];
		if (params->idcode1 == idcode[2])
			break;
	}

	if (i == ARRAY_SIZE(sst_spi_flash_table)) {
		debug("SF: Unsupported SST ID %02x\n", idcode[1]);
		return NULL;
	}

	stm = spi_flash_alloc(struct sst_spi_flash, spi, params->name);
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;

	if (stm->params->flags & SST_FEAT_WP)
		stm->flash.write = sst_write_wp;
	stm->flash.page_size = 256;
	stm->flash.sector_size = 4096;
	stm->flash.size = stm->flash.sector_size * params->nr_sectors;

	/* Flash powers up read-only, so clear BP# bits */
	spi_flash_cmd_write_status(&stm->flash, 0);

	return &stm->flash;
}
