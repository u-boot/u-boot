/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __XILINX_ZYNQMP_H
#define __XILINX_ZYNQMP_H

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_GENERIC_BOARD

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF9010000
#define GICC_BASE	0xF9020000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_SDRAM_SIZE

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_IDENT_STRING		" Xilinx ZynqMP"

#define CONFIG_SYS_TEXT_BASE		0x8000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* Generic Timer Definitions - setup in EL3. Setup by ATF for other cases */
#define COUNTER_FREQUENCY		4000000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x400000)

/* Serial setup */
#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SERIAL

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MEMORY
#define CONFIG_DOS_PARTITION

#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
#endif

#if defined(CONFIG_ZYNQ_SDHCI)
# define CONFIG_FAT_WRITE
# define CONFIG_CMD_EXT4_WRITE
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=0x80000\0" \
	"fdt_addr=0x7000000\0" \
	"fdt_high=0x10000000\0" \
	"sdboot=mmcinfo && fatload mmc 0:0 $fdt_addr system.dtb && " \
		"fatload mmc 0:0 $kernel_addr Image && booti $kernel_addr - $fdt_addr\0"

#define CONFIG_BOOTARGS		"setenv bootargs console=ttyPS0,${baudrate} " \
				"earlycon=cdns,mmio,0xff000000,${baudrate}n8"
#define CONFIG_PREBOOT		"run bootargs"
#define CONFIG_BOOTCOMMAND	"run $modeboot"
#define CONFIG_BOOTDELAY	5

#define CONFIG_BOARD_LATE_INIT

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_PROMPT		"ZynqMP> "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64

#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE       /* enable fit_format_{error,warning}() */

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_UNZIP

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_CLOCKS

#endif /* __XILINX_ZYNQMP_H */
