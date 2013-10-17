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

#include "sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:		Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:		Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:		Device ext_jedec ID
 * @sector_size:	Sector size of this device
 * @nr_sectors:		No.of sectors on this device
 * @flags:		Importent param, for flash specific behaviour
 */
struct spi_flash_params {
	const char *name;
	u32 jedec;
	u16 ext_jedec;
	u32 sector_size;
	u32 nr_sectors;
	u16 flags;
};

static const struct spi_flash_params spi_flash_params_table[] = {
#ifdef CONFIG_SPI_FLASH_ATMEL		/* ATMEL */
	{"AT45DB011D",	   0x1f2200, 0x0,	64 * 1024,     4,	       SECT_4K},
	{"AT45DB021D",	   0x1f2300, 0x0,	64 * 1024,     8,	       SECT_4K},
	{"AT45DB041D",	   0x1f2400, 0x0,	64 * 1024,     8,	       SECT_4K},
	{"AT45DB081D",	   0x1f2500, 0x0,	64 * 1024,    16,	       SECT_4K},
	{"AT45DB161D",	   0x1f2600, 0x0,	64 * 1024,    32,	       SECT_4K},
	{"AT45DB321D",	   0x1f2700, 0x0,	64 * 1024,    64,	       SECT_4K},
	{"AT45DB641D",	   0x1f2800, 0x0,	64 * 1024,   128,	       SECT_4K},
	{"AT25DF321",      0x1f4701, 0x0,	64 * 1024,    64,	       SECT_4K},
#endif
#ifdef CONFIG_SPI_FLASH_EON		/* EON */
	{"EN25Q32B",	   0x1c3016, 0x0,	64 * 1024,    64,	             0},
	{"EN25Q64",	   0x1c3017, 0x0,	64 * 1024,   128,	       SECT_4K},
	{"EN25Q128B",	   0x1c3018, 0x0,       64 * 1024,   256,	             0},
	{"EN25S64",	   0x1c3817, 0x0,	64 * 1024,   128,		     0},
#endif
#ifdef CONFIG_SPI_FLASH_GIGADEVICE	/* GIGADEVICE */
	{"GD25Q64B",	   0xc84017, 0x0,	64 * 1024,   128,	       SECT_4K},
	{"GD25LQ32",	   0xc86016, 0x0,	64 * 1024,    64,	       SECT_4K},
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX	/* MACRONIX */
	{"MX25L4005",	   0xc22013, 0x0,	64 * 1024,     8,	             0},
	{"MX25L8005",	   0xc22014, 0x0,	64 * 1024,    16,	             0},
	{"MX25L1605D",	   0xc22015, 0x0,	64 * 1024,    32,	             0},
	{"MX25L3205D",	   0xc22016, 0x0,	64 * 1024,    64,	             0},
	{"MX25L6405D",	   0xc22017, 0x0,	64 * 1024,   128,	             0},
	{"MX25L12805",	   0xc22018, 0x0,	64 * 1024,   256,	             0},
	{"MX25L25635F",	   0xc22019, 0x0,	64 * 1024,   512,	             0},
	{"MX25L51235F",	   0xc2201A, 0x0,	64 * 1024,  1024,	             0},
	{"MX25L12855E",	   0xc22618, 0x0,	64 * 1024,   256,	             0},
#endif
#ifdef CONFIG_SPI_FLASH_SPANSION	/* SPANSION */
	{"S25FL008A",	   0x010213, 0x0,	64 * 1024,    16,	             0},
	{"S25FL016A",	   0x010214, 0x0,	64 * 1024,    32,	             0},
	{"S25FL032A",	   0x010215, 0x0,	64 * 1024,    64,	             0},
	{"S25FL064A",	   0x010216, 0x0,	64 * 1024,   128,	             0},
	{"S25FL128P_256K", 0x012018, 0x0300,   256 * 1024,    64,	             0},
	{"S25FL128P_64K",  0x012018, 0x0301,    64 * 1024,   256,	             0},
	{"S25FL032P",	   0x010215, 0x4d00,    64 * 1024,    64,	             0},
	{"S25FL064P",	   0x010216, 0x4d00,    64 * 1024,   128,	             0},
	{"S25FL128S_64K",  0x012018, 0x4d01,    64 * 1024,   256,		     0},
	{"S25FL256S_256K", 0x010219, 0x4d00,    64 * 1024,   512,	             0},
	{"S25FL256S_64K",  0x010219, 0x4d01,    64 * 1024,   512,	             0},
	{"S25FL512S_256K", 0x010220, 0x4d00,    64 * 1024,  1024,	             0},
	{"S25FL512S_64K",  0x010220, 0x4d01,    64 * 1024,  1024,	             0},
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO		/* STMICRO */
	{"M25P10",	   0x202011, 0x0,       32 * 1024,     4,	             0},
	{"M25P20",	   0x202012, 0x0,       64 * 1024,     4,	             0},
	{"M25P40",	   0x202013, 0x0,       64 * 1024,     8,	             0},
	{"M25P80",	   0x202014, 0x0,       64 * 1024,    16,	             0},
	{"M25P16",	   0x202015, 0x0,       64 * 1024,    32,	             0},
	{"M25P32",	   0x202016, 0x0,       64 * 1024,    64,	             0},
	{"M25P64",	   0x202017, 0x0,       64 * 1024,   128,	             0},
	{"M25P128",	   0x202018, 0x0,      256 * 1024,    64,	             0},
	{"N25Q32",	   0x20ba16, 0x0,       64 * 1024,    64,	       SECT_4K},
	{"N25Q32A",	   0x20bb16, 0x0,       64 * 1024,    64,	       SECT_4K},
	{"N25Q64",	   0x20ba17, 0x0,       64 * 1024,   128,	       SECT_4K},
	{"N25Q64A",	   0x20bb17, 0x0,       64 * 1024,   128,	       SECT_4K},
	{"N25Q128",	   0x20ba18, 0x0,       64 * 1024,   256,	       SECT_4K},
	{"N25Q128A",	   0x20bb18, 0x0,       64 * 1024,   256,	       SECT_4K},
	{"N25Q256",	   0x20ba19, 0x0,       64 * 1024,   512,	       SECT_4K},
	{"N25Q256A",	   0x20bb19, 0x0,       64 * 1024,   512,	       SECT_4K},
	{"N25Q512",	   0x20ba20, 0x0,       64 * 1024,  1024,      E_FSR | SECT_4K},
	{"N25Q512A",	   0x20bb20, 0x0,       64 * 1024,  1024,      E_FSR | SECT_4K},
	{"N25Q1024",	   0x20ba21, 0x0,       64 * 1024,  2048,      E_FSR | SECT_4K},
	{"N25Q1024A",	   0x20bb21, 0x0,       64 * 1024,  2048,      E_FSR | SECT_4K},
#endif
#ifdef CONFIG_SPI_FLASH_SST		/* SST */
	{"SST25VF040B",	   0xbf258d, 0x0,	64 * 1024,     8,     SECT_4K | SST_WP},
	{"SST25VF080B",	   0xbf258e, 0x0,	64 * 1024,    16,     SECT_4K | SST_WP},
	{"SST25VF016B",	   0xbf2541, 0x0,	64 * 1024,    32,     SECT_4K | SST_WP},
	{"SST25VF032B",	   0xbf254a, 0x0,	64 * 1024,    64,     SECT_4K | SST_WP},
	{"SST25VF064C",	   0xbf254b, 0x0,	64 * 1024,   128,	       SECT_4K},
	{"SST25WF512",	   0xbf2501, 0x0,	64 * 1024,     1,     SECT_4K | SST_WP},
	{"SST25WF010",	   0xbf2502, 0x0,	64 * 1024,     2,     SECT_4K | SST_WP},
	{"SST25WF020",	   0xbf2503, 0x0,	64 * 1024,     4,     SECT_4K | SST_WP},
	{"SST25WF040",	   0xbf2504, 0x0,	64 * 1024,     8,     SECT_4K | SST_WP},
	{"SST25WF080",	   0xbf2505, 0x0,	64 * 1024,    16,     SECT_4K | SST_WP},
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND		/* WINBOND */
	{"W25P80",	   0xef2014, 0x0,	64 * 1024,    16,		    0},
	{"W25P16",	   0xef2015, 0x0,	64 * 1024,    32,		    0},
	{"W25P32",	   0xef2016, 0x0,	64 * 1024,    64,		    0},
	{"W25X40",	   0xef3013, 0x0,	64 * 1024,     8,	      SECT_4K},
	{"W25X16",	   0xef3015, 0x0,	64 * 1024,    32,	      SECT_4K},
	{"W25X32",	   0xef3016, 0x0,	64 * 1024,    64,	      SECT_4K},
	{"W25X64",	   0xef3017, 0x0,	64 * 1024,   128,	      SECT_4K},
	{"W25Q80BL",	   0xef4014, 0x0,	64 * 1024,    16,	      SECT_4K},
	{"W25Q16CL",	   0xef4015, 0x0,	64 * 1024,    32,	      SECT_4K},
	{"W25Q32BV",	   0xef4016, 0x0,	64 * 1024,    64,	      SECT_4K},
	{"W25Q64CV",	   0xef4017, 0x0,	64 * 1024,   128,	      SECT_4K},
	{"W25Q128BV",	   0xef4018, 0x0,	64 * 1024,   256,	      SECT_4K},
	{"W25Q256",	   0xef4019, 0x0,	64 * 1024,   512,	      SECT_4K},
	{"W25Q80BW",	   0xef5014, 0x0,	64 * 1024,    16,	      SECT_4K},
	{"W25Q16DW",	   0xef6015, 0x0,	64 * 1024,    32,	      SECT_4K},
	{"W25Q32DW",	   0xef6016, 0x0,	64 * 1024,    64,	      SECT_4K},
	{"W25Q64DW",	   0xef6017, 0x0,	64 * 1024,   128,	      SECT_4K},
	{"W25Q128FW",	   0xef6018, 0x0,	64 * 1024,   256,	      SECT_4K},
#endif
	/*
	 * Note:
	 * Below paired flash devices has similar spi_flash params.
	 * (S25FL129P_64K, S25FL128S_64K)
	 * (W25Q80BL, W25Q80BV)
	 * (W25Q16CL, W25Q16DV)
	 * (W25Q32BV, W25Q32FV_SPI)
	 * (W25Q64CV, W25Q64FV_SPI)
	 * (W25Q128BV, W25Q128FV_SPI)
	 * (W25Q32DW, W25Q32FV_QPI)
	 * (W25Q64DW, W25Q64FV_QPI)
	 * (W25Q128FW, W25Q128FV_QPI)
	 */
};

static struct spi_flash *spi_flash_validate_params(struct spi_slave *spi,
		u8 *idcode)
{
	const struct spi_flash_params *params;
	struct spi_flash *flash;
	int i;
	u16 jedec = idcode[1] << 8 | idcode[2];
	u16 ext_jedec = idcode[3] << 8 | idcode[4];

	/* Get the flash id (jedec = manuf_id + dev_id, ext_jedec) */
	for (i = 0; i < ARRAY_SIZE(spi_flash_params_table); i++) {
		params = &spi_flash_params_table[i];
		if ((params->jedec >> 16) == idcode[0]) {
			if ((params->jedec & 0xFFFF) == jedec) {
				if (params->ext_jedec == 0)
					break;
				else if (params->ext_jedec == ext_jedec)
					break;
			}
		}
	}

	if (i == ARRAY_SIZE(spi_flash_params_table)) {
		printf("SF: Unsupported flash IDs: ");
		printf("manuf %02x, jedec %04x, ext_jedec %04x\n",
		       idcode[0], jedec, ext_jedec);
		return NULL;
	}

	flash = malloc(sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate spi_flash\n");
		return NULL;
	}
	memset(flash, '\0', sizeof(*flash));

	/* Assign spi data */
	flash->spi = spi;
	flash->name = params->name;
	flash->memory_map = spi->memory_map;

	/* Assign spi_flash ops */
	flash->write = spi_flash_cmd_write_ops;
#ifdef CONFIG_SPI_FLASH_SST
	if (params->flags & SST_WP)
		flash->write = sst_write_wp;
#endif
	flash->erase = spi_flash_cmd_erase_ops;
	flash->read = spi_flash_cmd_read_ops;

	/* Compute the flash size */
	flash->page_size = (ext_jedec == 0x4d00) ? 512 : 256;
	flash->sector_size = params->sector_size;
	flash->size = flash->sector_size * params->nr_sectors;

	/* Compute erase sector and command */
	if (params->flags & SECT_4K) {
		flash->erase_cmd = CMD_ERASE_4K;
		flash->erase_size = 4096;
	} else if (params->flags & SECT_32K) {
		flash->erase_cmd = CMD_ERASE_32K;
		flash->erase_size = 32768;
	} else {
		flash->erase_cmd = CMD_ERASE_64K;
		flash->erase_size = flash->sector_size;
	}

	/* Poll cmd seclection */
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
	flash->memory_map = (void *)addr;

	return 0;
}
#endif /* CONFIG_OF_CONTROL */

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	struct spi_flash *flash = NULL;
	u8 idcode[5];
	int ret;

	/* Setup spi_slave */
	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
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
	if (flash->size > SPI_FLASH_16MB_BOUN) {
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

void spi_flash_free(struct spi_flash *flash)
{
	spi_free_slave(flash->spi);
	free(flash);
}
