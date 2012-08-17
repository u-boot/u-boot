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

/* I2C Configuration */
#define CONFIG_I2C
#define CONFIG_CMD_I2C
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_DRIVER_OMAP24XX_I2C

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

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_TEXT_BASE		0x402F0400
#define CONFIG_SPL_MAX_SIZE		(46 * 1024)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80800000
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

/* Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ

#endif	/* ! __CONFIG_AM335X_EVM_H */
