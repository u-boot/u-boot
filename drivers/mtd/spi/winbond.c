/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* M25Pxx-specific commands */
#define CMD_W25_WREN		0x06	/* Write Enable */
#define CMD_W25_WRDI		0x04	/* Write Disable */
#define CMD_W25_RDSR		0x05	/* Read Status Register */
#define CMD_W25_WRSR		0x01	/* Write Status Register */
#define CMD_W25_READ		0x03	/* Read Data Bytes */
#define CMD_W25_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_W25_PP		0x02	/* Page Program */
#define CMD_W25_SE		0x20	/* Sector (4K) Erase */
#define CMD_W25_BE		0xd8	/* Block (64K) Erase */
#define CMD_W25_CE		0xc7	/* Chip Erase */
#define CMD_W25_DP		0xb9	/* Deep Power-down */
#define CMD_W25_RES		0xab	/* Release from DP, and Read Signature */

#define WINBOND_ID_W25X16		0x3015
#define WINBOND_ID_W25X32		0x3016
#define WINBOND_ID_W25X64		0x3017

#define WINBOND_SR_WIP		(1 << 0)	/* Write-in-Progress */

struct winbond_spi_flash_params {
	uint16_t	id;
	/* Log2 of page size in power-of-two mode */
	uint8_t		l2_page_size;
	uint16_t	pages_per_sector;
	uint16_t	sectors_per_block;
	uint8_t		nr_blocks;
	const char	*name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct winbond_spi_flash {
	struct spi_flash flash;
	const struct winbond_spi_flash_params *params;
};

static inline struct winbond_spi_flash *
to_winbond_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct winbond_spi_flash, flash);
}

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= WINBOND_ID_W25X16,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "W25X16",
	},
	{
		.id			= WINBOND_ID_W25X32,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 64,
		.name			= "W25X32",
	},
	{
		.id			= WINBOND_ID_W25X64,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25X64",
	},
};

static int winbond_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 status;
	u8 cmd[4] = { CMD_W25_RDSR, 0xff, 0xff, 0xff };

	ret = spi_xfer(spi, 32, &cmd[0], NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret) {
			debug("SF: Failed to get status for cmd %02x: %d\n", cmd, ret);
			return -1;
		}

		if ((status & WINBOND_SR_WIP) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & WINBOND_SR_WIP) == 0)
		return 0;

	debug("SF: Timed out on command %02x: %d\n", cmd, ret);
	/* Timed out */
	return -1;
}

/*
 * Assemble the address part of a command for Winbond devices in
 * non-power-of-two page size mode.
 */
static void winbond_build_address(struct winbond_spi_flash *stm, u8 *cmd, u32 offset)
{
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;

	/*
	 * The "extra" space per page is the power-of-two page size
	 * divided by 32.
	 */
	page_shift = stm->params->l2_page_size;
	page_size = (1 << page_shift);
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	cmd[0] = page_addr >> (16 - page_shift);
	cmd[1] = page_addr << (page_shift - 8) | (byte_addr >> 8);
	cmd[2] = byte_addr;
}

static int winbond_read_fast(struct spi_flash *flash,
		u32 offset, size_t len, void *buf)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	u8 cmd[5];

	cmd[0] = CMD_READ_ARRAY_FAST;
	winbond_build_address(stm, cmd + 1, offset);
	cmd[4] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

static int winbond_write(struct spi_flash *flash,
		u32 offset, size_t len, const void *buf)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	page_shift = stm->params->l2_page_size;
	page_size = (1 << page_shift);
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = CMD_W25_PP;
		cmd[1] = page_addr >> (16 - page_shift);
		cmd[2] = page_addr << (page_shift - 8) | (byte_addr >> 8);
		cmd[3] = byte_addr;
		debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
			buf + actual,
			cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
				buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: Winbond Page Program failed\n");
			goto out;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: Winbond page programming timed out\n");
			goto out;
		}

		page_addr++;
		byte_addr = 0;
	}

	debug("SF: Winbond: Successfully programmed %u bytes @ 0x%x\n",
			len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

int winbond_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	unsigned long sector_size;
	unsigned int page_shift;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	page_shift = stm->params->l2_page_size;
	sector_size = (1 << page_shift) * stm->params->pages_per_sector;

	if (offset % sector_size || len % sector_size) {
		debug("SF: Erase offset/length not multiple of sector size\n");
		return -1;
	}

	len /= sector_size;
	cmd[0] = CMD_W25_SE;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual++) {
		winbond_build_address(stm, &cmd[1], offset + actual * sector_size);
		printf("Erase: %02x %02x %02x %02x\n",
				cmd[0], cmd[1], cmd[2], cmd[3]);

		ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: Winbond sector erase failed\n");
			goto out;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: Winbond sector erase timed out\n");
			goto out;
		}
	}

	debug("SF: Winbond: Successfully erased %u bytes @ 0x%x\n",
			len * sector_size, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode)
{
	const struct winbond_spi_flash_params *params;
	unsigned page_size;
	struct winbond_spi_flash *stm;
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

	stm = malloc(sizeof(struct winbond_spi_flash));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = 1 << params->l2_page_size;

	stm->flash.write = winbond_write;
	stm->flash.erase = winbond_erase;
	stm->flash.read = winbond_read_fast;
	stm->flash.size = page_size * params->pages_per_sector
				* params->sectors_per_block
				* params->nr_blocks;

	printf("SF: Detected %s with page size %u, total ",
	       params->name, page_size);
	print_size(stm->flash.size, "\n");

	return &stm->flash;
}
