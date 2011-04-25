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

#define CMD_SST_WREN		0x06	/* Write Enable */
#define CMD_SST_WRDI		0x04	/* Write Disable */
#define CMD_SST_RDSR		0x05	/* Read Status Register */
#define CMD_SST_WRSR		0x01	/* Write Status Register */
#define CMD_SST_READ		0x03	/* Read Data Bytes */
#define CMD_SST_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_SST_BP		0x02	/* Byte Program */
#define CMD_SST_AAI_WP		0xAD	/* Auto Address Increment Word Program */
#define CMD_SST_SE		0x20	/* Sector Erase */

#define SST_SR_WIP		(1 << 0)	/* Write-in-Progress */
#define SST_SR_WEL		(1 << 1)	/* Write enable */
#define SST_SR_BP0		(1 << 2)	/* Block Protection 0 */
#define SST_SR_BP1		(1 << 3)	/* Block Protection 1 */
#define SST_SR_BP2		(1 << 4)	/* Block Protection 2 */
#define SST_SR_AAI		(1 << 6)	/* Addressing mode */
#define SST_SR_BPL		(1 << 7)	/* BP bits lock */

struct sst_spi_flash_params {
	u8 idcode1;
	u16 nr_sectors;
	const char *name;
};

struct sst_spi_flash {
	struct spi_flash flash;
	const struct sst_spi_flash_params *params;
};

static inline struct sst_spi_flash *to_sst_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct sst_spi_flash, flash);
}

#define SST_SECTOR_SIZE (4 * 1024)
static const struct sst_spi_flash_params sst_spi_flash_table[] = {
	{
		.idcode1 = 0x8d,
		.nr_sectors = 128,
		.name = "SST25VF040B",
	},{
		.idcode1 = 0x8e,
		.nr_sectors = 256,
		.name = "SST25VF080B",
	},{
		.idcode1 = 0x41,
		.nr_sectors = 512,
		.name = "SST25VF016B",
	},{
		.idcode1 = 0x4a,
		.nr_sectors = 1024,
		.name = "SST25VF032B",
	},{
		.idcode1 = 0x4b,
		.nr_sectors = 2048,
		.name = "SST25VF064C",
	},{
		.idcode1 = 0x01,
		.nr_sectors = 16,
		.name = "SST25WF512",
	},{
		.idcode1 = 0x02,
		.nr_sectors = 32,
		.name = "SST25WF010",
	},{
		.idcode1 = 0x03,
		.nr_sectors = 64,
		.name = "SST25WF020",
	},{
		.idcode1 = 0x04,
		.nr_sectors = 128,
		.name = "SST25WF040",
	},
};

static int
sst_enable_writing(struct spi_flash *flash)
{
	int ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		debug("SF: Enabling Write failed\n");
	return ret;
}

static int
sst_disable_writing(struct spi_flash *flash)
{
	int ret = spi_flash_cmd_write_disable(flash);
	if (ret)
		debug("SF: Disabling Write failed\n");
	return ret;
}

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
		spi_w8r8(flash->spi, CMD_SST_RDSR), buf, cmd[0], offset);

	ret = sst_enable_writing(flash);
	if (ret)
		return ret;

	ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), buf, 1);
	if (ret)
		return ret;

	return spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
}

static int
sst_write(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
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

	ret = sst_enable_writing(flash);
	if (ret)
		goto done;

	cmd_len = 4;
	cmd[0] = CMD_SST_AAI_WP;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset;

	for (; actual < len - 1; actual += 2) {
		debug("WP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
		     spi_w8r8(flash->spi, CMD_SST_RDSR), buf + actual, cmd[0],
		     offset);

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
		ret = sst_disable_writing(flash);

	/* If there is a single trailing byte, write it out */
	if (!ret && actual != len)
		ret = sst_byte_write(flash, offset, buf + actual);

 done:
	debug("SF: sst: program %s %zu bytes @ 0x%zx\n",
	      ret ? "failure" : "success", len, offset - actual);

	spi_release_bus(flash->spi);
	return ret;
}

static int sst_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_SST_SE, offset, len);
}

static int
sst_unlock(struct spi_flash *flash)
{
	int ret;
	u8 cmd, status;

	ret = sst_enable_writing(flash);
	if (ret)
		return ret;

	cmd = CMD_SST_WRSR;
	status = 0;
	ret = spi_flash_cmd_write(flash->spi, &cmd, 1, &status, 1);
	if (ret)
		debug("SF: Unable to set status byte\n");

	debug("SF: sst: status = %x\n", spi_w8r8(flash->spi, CMD_SST_RDSR));

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

	stm = malloc(sizeof(*stm));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	stm->flash.write = sst_write;
	stm->flash.erase = sst_erase;
	stm->flash.read = spi_flash_cmd_read_fast;
	stm->flash.sector_size = SST_SECTOR_SIZE;
	stm->flash.size = stm->flash.sector_size * params->nr_sectors;

	/* Flash powers up read-only, so clear BP# bits */
	sst_unlock(&stm->flash);

	return &stm->flash;
}
