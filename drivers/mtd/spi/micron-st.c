// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info micron_nor_parts[] = {
	{ INFO("n25q016a",	 0x20bb15, 0, 64 * 1024,   32, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q032",	 0x20ba16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ INFO("n25q032a",	0x20bb16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ INFO("n25q064",     0x20ba17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q064a",    0x20bb17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q128a11",  0x20bb18, 0, 64 * 1024,  256, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q128a13",  0x20ba18, 0, 64 * 1024,  256, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25ql256a",    0x20ba19, 0x104400, 64 * 1024,  512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | USE_FSR) },
	{ INFO("n25q256a",    0x20ba19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_FSR) },
	{ INFO6("mt25qu256a",  0x20bb19, 0x104400, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | USE_FSR) },
	{ INFO("n25q256ax1",  0x20bb19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ | USE_FSR) },
	{ INFO("mt25qu128ab", 0x20bb18, 0, 64 * 1024,  256, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25qu512a",  0x20bb20, 0x104400, 64 * 1024, 1024,
		 SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		 USE_FSR) },
	{ INFO("n25q512a",    0x20bb20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25ql512a",  0x20ba20, 0x104400, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("n25q512ax3",  0x20ba20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO("n25q00",      0x20ba21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("n25q00a",     0x20bb21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("mt25ql01g",   0x21ba20, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO6("mt25qu01g",  0x20bb21, 0x104400, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE | SPI_NOR_4B_OPCODES) },
	{ INFO("mt25qu02g",   0x20bb22, 0, 64 * 1024, 4096, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("mt25ql02g",   0x20ba22, 0, 64 * 1024, 4096, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE | SPI_NOR_4B_OPCODES) },
#ifdef CONFIG_SPI_FLASH_MT35XU
	{ INFO("mt35xl512aba", 0x2c5a1a, 0,  128 * 1024,  512, USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("mt35xu512aba", 0x2c5b1a, 0,  128 * 1024,  512, USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("mt35xu01gaba", 0x2c5b1b, 0,  128 * 1024,  1024,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
#endif /* CONFIG_SPI_FLASH_MT35XU */
	{ INFO6("mt35xu01g",  0x2c5b1b, 0x104100, 128 * 1024,  1024,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
	{ INFO("mt35xu02g",  0x2c5b1c, 0, 128 * 1024,  2048,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
};

static const struct flash_info st_nor_parts[] = {
	{ INFO("m25p10",  0x202011,  0,  32 * 1024,   4, 0) },
	{ INFO("m25p20",  0x202012,  0,  64 * 1024,   4, 0) },
	{ INFO("m25p40",  0x202013,  0,  64 * 1024,   8, 0) },
	{ INFO("m25p80",  0x202014,  0,  64 * 1024,  16, 0) },
	{ INFO("m25p16",  0x202015,  0,  64 * 1024,  32, 0) },
	{ INFO("m25p32",  0x202016,  0,  64 * 1024,  64, 0) },
	{ INFO("m25p64",  0x202017,  0,  64 * 1024, 128, 0) },
	{ INFO("m25p128", 0x202018,  0, 256 * 1024,  64, 0) },
	{ INFO("m25pe16", 0x208015,  0, 64 * 1024, 32, SECT_4K) },
	{ INFO("m25px16",    0x207115,  0, 64 * 1024, 32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("m25px64",    0x207117,  0, 64 * 1024, 128, 0) },
};

const struct spi_nor_manufacturer spi_nor_micron = {
	.parts = micron_nor_parts,
	.nparts = ARRAY_SIZE(micron_nor_parts),
};

const struct spi_nor_manufacturer spi_nor_st = {
	.parts = st_nor_parts,
	.nparts = ARRAY_SIZE(st_nor_parts),
};
