/*
 * SPI flash probing
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>

#include "sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

/* Read commands array */
static u8 spi_read_cmds_array[] = {
	CMD_READ_ARRAY_SLOW,
	CMD_READ_DUAL_OUTPUT_FAST,
	CMD_READ_DUAL_IO_FAST,
	CMD_READ_QUAD_OUTPUT_FAST,
	CMD_READ_QUAD_IO_FAST,
};

#ifdef CONFIG_SPI_FLASH_MACRONIX
static int spi_flash_set_qeb_mxic(struct spi_flash *flash)
{
	u8 qeb_status;
	int ret;

	ret = spi_flash_cmd_read_status(flash, &qeb_status);
	if (ret < 0)
		return ret;

	if (qeb_status & STATUS_QEB_MXIC) {
		debug("SF: mxic: QEB is already set\n");
	} else {
		ret = spi_flash_cmd_write_status(flash, STATUS_QEB_MXIC);
		if (ret < 0)
			return ret;
	}

	return ret;
}
#endif

#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
static int spi_flash_set_qeb_winspan(struct spi_flash *flash)
{
	u8 qeb_status;
	int ret;

	ret = spi_flash_cmd_read_config(flash, &qeb_status);
	if (ret < 0)
		return ret;

	if (qeb_status & STATUS_QEB_WINSPAN) {
		debug("SF: winspan: QEB is already set\n");
	} else {
		ret = spi_flash_cmd_write_config(flash, STATUS_QEB_WINSPAN);
		if (ret < 0)
			return ret;
	}

	return ret;
}
#endif

static int spi_flash_set_qeb(struct spi_flash *flash, u8 idcode0)
{
	switch (idcode0) {
#ifdef CONFIG_SPI_FLASH_MACRONIX
	case SPI_FLASH_CFI_MFR_MACRONIX:
		return spi_flash_set_qeb_mxic(flash);
#endif
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
	case SPI_FLASH_CFI_MFR_SPANSION:
	case SPI_FLASH_CFI_MFR_WINBOND:
		return spi_flash_set_qeb_winspan(flash);
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO
	case SPI_FLASH_CFI_MFR_STMICRO:
		debug("SF: QEB is volatile for %02x flash\n", idcode0);
		return 0;
#endif
	default:
		printf("SF: Need set QEB func for %02x flash\n", idcode0);
		return -1;
	}
}

static struct spi_flash *spi_flash_validate_params(struct spi_slave *spi,
		u8 *idcode)
{
	const struct spi_flash_params *params;
	struct spi_flash *flash;
	u8 cmd;
	u16 jedec = idcode[1] << 8 | idcode[2];
	u16 ext_jedec = idcode[3] << 8 | idcode[4];

	params = spi_flash_params_table;
	for (; params->name != NULL; params++) {
		if ((params->jedec >> 16) == idcode[0]) {
			if ((params->jedec & 0xFFFF) == jedec) {
				if (params->ext_jedec == 0)
					break;
				else if (params->ext_jedec == ext_jedec)
					break;
			}
		}
	}

	if (!params->name) {
		printf("SF: Unsupported flash IDs: ");
		printf("manuf %02x, jedec %04x, ext_jedec %04x\n",
		       idcode[0], jedec, ext_jedec);
		return NULL;
	}

	flash = calloc(1, sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate spi_flash\n");
		return NULL;
	}

	/* Assign spi data */
	flash->spi = spi;
	flash->name = params->name;
	flash->memory_map = spi->memory_map;
	flash->dual_flash = flash->spi->option;

	/* Assign spi_flash ops */
	flash->write = spi_flash_cmd_write_ops;
#ifdef CONFIG_SPI_FLASH_SST
	if (params->flags & SST_WP)
		flash->write = sst_write_wp;
#endif
	flash->erase = spi_flash_cmd_erase_ops;
	flash->read = spi_flash_cmd_read_ops;

	/* Compute the flash size */
	flash->shift = (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) ? 1 : 0;
	/*
	 * The Spansion S25FL032P and S25FL064P have 256b pages, yet use the
	 * 0x4d00 Extended JEDEC code. The rest of the Spansion flashes with
	 * the 0x4d00 Extended JEDEC code have 512b pages. All of the others
	 * have 256b pages.
	 */
	if (ext_jedec == 0x4d00) {
		if ((jedec == 0x0215) || (jedec == 0x216))
			flash->page_size = 256;
		else
			flash->page_size = 512;
	} else {
		flash->page_size = 256;
	}
	flash->page_size <<= flash->shift;
	flash->sector_size = params->sector_size << flash->shift;
	flash->size = flash->sector_size * params->nr_sectors << flash->shift;
#ifdef CONFIG_SF_DUAL_FLASH
	if (flash->dual_flash & SF_DUAL_STACKED_FLASH)
		flash->size <<= 1;
#endif

	/* Compute erase sector and command */
	if (params->flags & SECT_4K) {
		flash->erase_cmd = CMD_ERASE_4K;
		flash->erase_size = 4096 << flash->shift;
	} else if (params->flags & SECT_32K) {
		flash->erase_cmd = CMD_ERASE_32K;
		flash->erase_size = 32768 << flash->shift;
	} else {
		flash->erase_cmd = CMD_ERASE_64K;
		flash->erase_size = flash->sector_size;
	}

	/* Look for the fastest read cmd */
	cmd = fls(params->e_rd_cmd & flash->spi->op_mode_rx);
	if (cmd) {
		cmd = spi_read_cmds_array[cmd - 1];
		flash->read_cmd = cmd;
	} else {
		/* Go for default supported read cmd */
		flash->read_cmd = CMD_READ_ARRAY_FAST;
	}

	/* Not require to look for fastest only two write cmds yet */
	if (params->flags & WR_QPP && flash->spi->op_mode_tx & SPI_OPM_TX_QPP)
		flash->write_cmd = CMD_QUAD_PAGE_PROGRAM;
	else
		/* Go for default supported write cmd */
		flash->write_cmd = CMD_PAGE_PROGRAM;

	/* Set the quad enable bit - only for quad commands */
	if ((flash->read_cmd == CMD_READ_QUAD_OUTPUT_FAST) ||
	    (flash->read_cmd == CMD_READ_QUAD_IO_FAST) ||
	    (flash->write_cmd == CMD_QUAD_PAGE_PROGRAM)) {
		if (spi_flash_set_qeb(flash, idcode[0])) {
			debug("SF: Fail to set QEB for %02x\n", idcode[0]);
			return NULL;
		}
	}

	/* Read dummy_byte: dummy byte is determined based on the
	 * dummy cycles of a particular command.
	 * Fast commands - dummy_byte = dummy_cycles/8
	 * I/O commands- dummy_byte = (dummy_cycles * no.of lines)/8
	 * For I/O commands except cmd[0] everything goes on no.of lines
	 * based on particular command but incase of fast commands except
	 * data all go on single line irrespective of command.
	 */
	switch (flash->read_cmd) {
	case CMD_READ_QUAD_IO_FAST:
		flash->dummy_byte = 2;
		break;
	case CMD_READ_ARRAY_SLOW:
		flash->dummy_byte = 0;
		break;
	default:
		flash->dummy_byte = 1;
	}

	/* Poll cmd selection */
	flash->poll_cmd = CMD_READ_STATUS;
#ifdef CONFIG_SPI_FLASH_STMICRO
	if (params->flags & E_FSR)
		flash->poll_cmd = CMD_FLAG_STATUS;
#endif

	/* Configure the BAR - discover bank cmds and read current bank */
#ifdef CONFIG_SPI_FLASH_BAR
	u8 curr_bank = 0;
	if (flash->size > SPI_FLASH_16MB_BOUN) {
		flash->bank_read_cmd = (idcode[0] == 0x01) ?
					CMD_BANKADDR_BRRD : CMD_EXTNADDR_RDEAR;
		flash->bank_write_cmd = (idcode[0] == 0x01) ?
					CMD_BANKADDR_BRWR : CMD_EXTNADDR_WREAR;

		if (spi_flash_read_common(flash, &flash->bank_read_cmd, 1,
					  &curr_bank, 1)) {
			debug("SF: fail to read bank addr register\n");
			return NULL;
		}
		flash->bank_curr = curr_bank;
	} else {
		flash->bank_curr = curr_bank;
	}
#endif

	/* Flash powers up read-only, so clear BP# bits */
#if defined(CONFIG_SPI_FLASH_ATMEL) || \
	defined(CONFIG_SPI_FLASH_MACRONIX) || \
	defined(CONFIG_SPI_FLASH_SST)
		spi_flash_cmd_write_status(flash, 0);
#endif

	return flash;
}

#ifdef CONFIG_OF_CONTROL
int spi_flash_decode_fdt(const void *blob, struct spi_flash *flash)
{
	fdt_addr_t addr;
	fdt_size_t size;
	int node;

	/* If there is no node, do nothing */
	node = fdtdec_next_compatible(blob, 0, COMPAT_GENERIC_SPI_FLASH);
	if (node < 0)
		return 0;

	addr = fdtdec_get_addr_size(blob, node, "memory-map", &size);
	if (addr == FDT_ADDR_T_NONE) {
		debug("%s: Cannot decode address\n", __func__);
		return 0;
	}

	if (flash->size != size) {
		debug("%s: Memory map must cover entire device\n", __func__);
		return -1;
	}
	flash->memory_map = map_sysmem(addr, size);

	return 0;
}
#endif /* CONFIG_OF_CONTROL */

static struct spi_flash *spi_flash_probe_slave(struct spi_slave *spi)
{
	struct spi_flash *flash = NULL;
	u8 idcode[5];
	int ret;

	/* Setup spi_slave */
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return NULL;
	}

	/* Claim spi bus */
	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
		goto err_claim_bus;
	}

	/* Read the ID codes */
	ret = spi_flash_cmd(spi, CMD_READ_ID, idcode, sizeof(idcode));
	if (ret) {
		printf("SF: Failed to get idcodes\n");
		goto err_read_id;
	}

#ifdef DEBUG
	printf("SF: Got idcodes\n");
	print_buffer(0, idcode, 1, sizeof(idcode), 0);
#endif

	/* Validate params from spi_flash_params table */
	flash = spi_flash_validate_params(spi, idcode);
	if (!flash)
		goto err_read_id;

#ifdef CONFIG_OF_CONTROL
	if (spi_flash_decode_fdt(gd->fdt_blob, flash)) {
		debug("SF: FDT decode error\n");
		goto err_read_id;
	}
#endif
#ifndef CONFIG_SPL_BUILD
	printf("SF: Detected %s with page size ", flash->name);
	print_size(flash->page_size, ", erase size ");
	print_size(flash->erase_size, ", total ");
	print_size(flash->size, "");
	if (flash->memory_map)
		printf(", mapped at %p", flash->memory_map);
	puts("\n");
#endif
#ifndef CONFIG_SPI_FLASH_BAR
	if (((flash->dual_flash == SF_SINGLE_FLASH) &&
	     (flash->size > SPI_FLASH_16MB_BOUN)) ||
	     ((flash->dual_flash > SF_SINGLE_FLASH) &&
	     (flash->size > SPI_FLASH_16MB_BOUN << 1))) {
		puts("SF: Warning - Only lower 16MiB accessible,");
		puts(" Full access #define CONFIG_SPI_FLASH_BAR\n");
	}
#endif

	/* Release spi bus */
	spi_release_bus(spi);

	return flash;

err_read_id:
	spi_release_bus(spi);
err_claim_bus:
	spi_free_slave(spi);
	return NULL;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	return spi_flash_probe_slave(spi);
}

#ifdef CONFIG_OF_SPI_FLASH
struct spi_flash *spi_flash_probe_fdt(const void *blob, int slave_node,
				      int spi_node)
{
	struct spi_slave *spi;

	spi = spi_setup_slave_fdt(blob, slave_node, spi_node);
	return spi_flash_probe_slave(spi);
}
#endif

void spi_flash_free(struct spi_flash *flash)
{
	spi_free_slave(flash->spi);
	free(flash);
}
