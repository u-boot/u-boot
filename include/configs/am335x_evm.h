/*
 * am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_AM335X_EVM_H
#define __CONFIG_AM335X_EVM_H

#define CONFIG_AM33XX
#define CONFIG_CMD_MEMORY	/* for mtest */
#undef CONFIG_GZIP
#undef CONFIG_ZLIB
#undef CONFIG_SYS_HUSH_PARSER
#undef CONFIG_CMD_NET

#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>

#define CONFIG_SETUP_PLL
#define CONFIG_AM335X_CONFIG_DDR
#define CONFIG_ENV_SIZE			0x400
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 * 1024))
#define CONFIG_SYS_PROMPT		"U-Boot# "
#define CONFIG_SYS_NO_FLASH
#define MACH_TYPE_TIAM335EVM		3589	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_TIAM335EVM

#define CONFIG_CMD_ASKENV
#define CONFIG_VERSION_VARIABLE

/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		3
#define CONFIG_SYS_AUTOLOAD		"no"
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"ramdisk_file=ramdisk.gz\0" \

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_CMD_ECHO

/* max number of command args */
#define CONFIG_SYS_MAXARGS		16

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * memtest works on 8 MB in DRAM after skipping 32MB from
 * start addr of ram disk
 */
#define CONFIG_SYS_MEMTEST_START	(PHYS_DRAM_1 + (64 * 1024 * 1024))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

#define CONFIG_SYS_LOAD_ADDR		0x81000000 /* Default load address */
#define CONFIG_SYS_HZ			1000 /* 1ms clock */

#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

 /* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1		/*  1 bank of DRAM */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE		0x10000000 /*(0x80000000 / 8) 256 MB */
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1GB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_DRAM_1
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x1000 - \
						GENERATED_GBL_DATA_SIZE)
 /* Platform/Board specific defs */
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1			1
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_CONSOLE_INFO_QUIET

#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_TEXT_BASE		0x402f0400

/* Unsupported features */
#undef CONFIG_USE_IRQ

#endif	/* ! __CONFIG_AM335X_EVM_H */
