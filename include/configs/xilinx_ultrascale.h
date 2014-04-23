/*
 * Configuration for Xilinx UltraScale MP
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __XILINX_ULTRASCALE_H
#define __XILINX_ULTRASCALE_H

#include <asm/arch/hardware.h>

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

#define CONFIG_SYS_NO_FLASH

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x40000000

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#define CONFIG_IDENT_STRING		" Xilinx UltraScale MP"
#define CONFIG_BOOTP_VCI_STRING		"U-boot.armv8.Xilinx_UltraScale_MP"

/* Text base on 16MB for now - 0 doesn't work */
#define CONFIG_SYS_TEXT_BASE		0x100000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

#define CONFIG_DEFAULT_DEVICE_TREE	ultrascale

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		0x1800000 /* 24MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/* Serial setup */
#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SERIAL

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

/* Initial environment variables */
#define CONFIG_BOOTARGS			"console=ttyPS0"
#define CONFIG_BOOTCOMMAND		"echo Hello Xilinx UltraScale MP"
#define CONFIG_BOOTDELAY		-1

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_PROMPT		"UltraScale> "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING		1
/* max command args */
#define CONFIG_SYS_MAXARGS		64

#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE       /* enable fit_format_{error,warning}() */

#endif /* __XILINX_ULTRASCALE_H */
