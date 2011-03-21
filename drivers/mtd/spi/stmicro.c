/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* M25Pxx-specific commands */
#define CMD_M25PXX_WREN		0x06	/* Write Enable */
#define CMD_M25PXX_WRDI		0x04	/* Write Disable */
#define CMD_M25PXX_RDSR		0x05	/* Read Status Register */
#define CMD_M25PXX_WRSR		0x01	/* Write Status Register */
#define CMD_M25PXX_READ		0x03	/* Read Data Bytes */
#define CMD_M25PXX_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_M25PXX_PP		0x02	/* Page Program */
#define CMD_M25PXX_SE		0xd8	/* Sector Erase */
#define CMD_M25PXX_BE		0xc7	/* Bulk Erase */
#define CMD_M25PXX_DP		0xb9	/* Deep Power-down */
#define CMD_M25PXX_RES		0xab	/* Release from DP, and Read Signature */

#define STM_ID_M25P10		0x11
#define STM_ID_M25P16		0x15
#define STM_ID_M25P20		0x12
#define STM_ID_M25P32		0x16
#define STM_ID_M25P40		0x13
#define STM_ID_M25P64		0x17
#define STM_ID_M25P80		0x14
#define STM_ID_M25P128		0x18

#define STM_ID_N25Q128		0x20BB1810

#define STMICRO_SR_WIP		(1 << 0)	/* Write-in-Progress */

struct stmicro_spi_flash_params {
	u8 idcode1;
	/* XILINX: idcode1 isn't specific enough;
	 * multiple non-compatible devices match. Store complete idcode */
	u32 idcode;
	u16 page_size;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct stmicro_spi_flash {
	struct spi_flash flash;
	const struct stmicro_spi_flash_params *params;

	unsigned addr_width;
};

static inline struct stmicro_spi_flash *to_stmicro_spi_flash(struct spi_flash
							     *flash)
{
	return container_of(flash, struct stmicro_spi_flash, flash);
}

static const struct stmicro_spi_flash_params stmicro_spi_flash_table[] = {
	{
		.idcode1 = STM_ID_M25P10,
		.page_size = 256,
		.pages_per_sector = 128,
		.nr_sectors = 4,
		.name = "M25P10",
	},
	{
		.idcode1 = STM_ID_M25P16,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "M25P16",
	},
	{
		.idcode1 = STM_ID_M25P20,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 4,
		.name = "M25P20",
	},
	{
		.idcode1 = STM_ID_M25P32,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "M25P32",
	},
	{
		.idcode1 = STM_ID_M25P40,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 8,
		.name = "M25P40",
	},
	{
		.idcode1 = STM_ID_M25P64,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "M25P64",
	},
	{
		.idcode1 = STM_ID_M25P80,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "M25P80",
	},
/*
 * XILINX:
 * This table only examines the capacity of the device.
 * The M25P128 and N25Q128 have the same capacity (16777216)
 * but different number of sectors, pages per sector.
 * Match the N25Q128 which is actually present on the board.
 */
	{
		.idcode1 = STM_ID_M25P128,
		.idcode = STM_ID_N25Q128,
		.page_size = 256,
		.pages_per_sector = 256,
#ifdef CONFIG_XILINX_PSS_QSPI_USE_DUAL_FLASH
		.nr_sectors = 512,
		.name = "N25Q128x2",
#else
		.nr_sectors = 256,
		.name = "N25Q128",
#endif
	},
	{
		.idcode1 = STM_ID_M25P128,
		.page_size = 256,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "M25P128",
	},
};

static inline void stmicro_addr2cmd(struct stmicro_spi_flash *stm,
                             unsigned page_addr, unsigned byte_addr, u8 *cmd)
{
	/* opcode is in cmd[0] */
	if (stm->addr_width == 4) {
		cmd[1] = page_addr >> 16;
		cmd[2] = page_addr >> 8;
		cmd[3] = page_addr;
		cmd[4] = byte_addr;
	} else {
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;
	}
}

static inline int stmicro_cmdsz(struct stmicro_spi_flash *flash)
{
	return 1 + flash->addr_width;
}

static int stmicro_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 cmd = CMD_M25PXX_RDSR;
	u8 status;

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if ((status & STMICRO_SR_WIP) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & STMICRO_SR_WIP) == 0)
		return 0;

	/* Timed out */
	return -1;
}

static int stmicro_read_fast(struct spi_flash *flash,
			     u32 offset, size_t len, void *buf)
{
	struct stmicro_spi_flash *stm = to_stmicro_spi_flash(flash);
	unsigned long page_addr;
	unsigned long page_size;
	u8 cmd[6];

	page_size = stm->params->page_size;
	page_addr = offset / page_size;

	cmd[0] = CMD_READ_ARRAY_FAST;
	stmicro_addr2cmd(stm, page_addr, offset % page_size, cmd);
	cmd[stmicro_cmdsz(stm)] = 0x00;

	return spi_flash_read_common(flash, cmd, stmicro_cmdsz(stm)+1 , buf, len);
}

static int stmicro_write(struct spi_flash *flash,
			 u32 offset, size_t len, const void *buf)
{
	struct stmicro_spi_flash *stm = to_stmicro_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[5];

	page_size = stm->params->page_size;
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

		cmd[0] = CMD_M25PXX_PP;
		stmicro_addr2cmd(stm, page_addr, byte_addr, cmd);

		debug
		    ("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
		     buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_M25PXX_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, stmicro_cmdsz(stm),
					  buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: STMicro Page Program failed\n");
			break;
		}

		ret = stmicro_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: STMicro page programming timed out\n");
			break;
		}

		page_addr++;
		byte_addr = 0;
	}

	debug("SF: STMicro: Successfully programmed %u bytes @ 0x%x\n",
	      len, offset);

	spi_release_bus(flash->spi);
	return ret;
}

int stmicro_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct stmicro_spi_flash *stm = to_stmicro_spi_flash(flash);
	unsigned long sector_size;
	size_t actual;
	int ret;
	u8 cmd[5];

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	sector_size = stm->params->page_size * stm->params->pages_per_sector;

	if (offset % sector_size || len % sector_size) {
		debug("SF: Erase offset/length not multiple of sector size\n");
		return -1;
	}

	len /= sector_size;
	cmd[0] = CMD_M25PXX_SE;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = 0;
	for (actual = 0; actual < len; actual++) {
		unsigned page_addr;

		page_addr = offset / stm->params->page_size;
		stmicro_addr2cmd(stm, page_addr, 0, cmd);

		offset += sector_size;

		ret = spi_flash_cmd(flash->spi, CMD_M25PXX_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, stmicro_cmdsz(stm), NULL, 0);
		if (ret < 0) {
			debug("SF: STMicro page erase failed\n");
			break;
		}

		ret = stmicro_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: STMicro page erase timed out\n");
			break;
		}
	}

	debug("SF: STMicro: Successfully erased %u bytes @ 0x%x\n",
	      len * sector_size, offset);

	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *spi_flash_probe_stmicro(struct spi_slave *spi, u8 * idcode)
{
	const struct stmicro_spi_flash_params *params;
	struct stmicro_spi_flash *stm;
	unsigned int i;

	if (idcode[0] == 0xff) {
		i = spi_flash_cmd(spi, CMD_M25PXX_RES,
				  idcode, 4);
		if (i)
			return NULL;
		if ((idcode[3] & 0xf0) == 0x10) {
			idcode[0] = 0x20;
			idcode[1] = 0x20;
			idcode[2] = idcode[3] + 1;
		} else
			return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(stmicro_spi_flash_table); i++) {
		params = &stmicro_spi_flash_table[i];
		if (params->idcode1 == idcode[2]) {
			if (params->idcode == 0) {
				break;
			}
			/* Check complete device ID, if specified */
			if ((((params->idcode & 0xFF000000) >> 24) == idcode[0]) &&
			    (((params->idcode & 0x00FF0000) >> 16) == idcode[1]) &&
			    (((params->idcode & 0x000000FF) >>  0) == idcode[3])) {
				break;
			}
		}
	}

	if (i == ARRAY_SIZE(stmicro_spi_flash_table)) {
		debug("SF: Unsupported STMicro ID %02x\n", idcode[1]);
		return NULL;
	}

	stm = malloc(sizeof(struct stmicro_spi_flash));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	stm->flash.write = stmicro_write;
	stm->flash.erase = stmicro_erase;
	stm->flash.read = stmicro_read_fast;
	stm->flash.size = params->page_size * params->pages_per_sector
	    * params->nr_sectors;

	if (stm->flash.size > 0x1000000) {
 		stm->addr_width = 4;
	} else {
		stm->addr_width = 3;
	} 

	printf("SF: Detected %s with page size %u, total ",
	       params->name, params->page_size);
	print_size(stm->flash.size, "\n");

	return &stm->flash;
}
