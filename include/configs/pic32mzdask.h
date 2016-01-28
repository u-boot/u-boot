/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Microchip PIC32MZ[DA] Starter Kit.
 */

#ifndef __PIC32MZDASK_CONFIG_H
#define __PIC32MZDASK_CONFIG_H

/* System Configuration */
#define CONFIG_SYS_TEXT_BASE		0x9d004000 /* .text */
#define CONFIG_DISPLAY_BOARDINFO

/*--------------------------------------------
 * CPU configuration
 */
/* CPU Timer rate */
#define CONFIG_SYS_MIPS_TIMER_FREQ	100000000

/* Cache Configuration */
#define CONFIG_SYS_MIPS_CACHE_MODE	CONF_CM_CACHABLE_NONCOHERENT

/*----------------------------------------------------------------------
 * Memory Layout
 */
#define CONFIG_SYS_SRAM_BASE		0x80000000
#define CONFIG_SYS_SRAM_SIZE		0x00080000 /* 512K */

/* Initial RAM for temporary stack, global data */
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#define CONFIG_SYS_INIT_RAM_ADDR	\
	(CONFIG_SYS_SRAM_BASE + CONFIG_SYS_SRAM_SIZE - CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE - 1)

/* SDRAM Configuration (for final code, data, stack, heap) */
#define CONFIG_SYS_SDRAM_BASE		0x88000000
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_BOOTPARAMS_LEN	(4 << 10)
#define CONFIG_STACKSIZE		(4 << 10) /* regular stack */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)

#define CONFIG_SYS_LOAD_ADDR		0x88500000 /* default load address */
#define CONFIG_SYS_ENV_ADDR		0x88300000

/* Memory Test */
#define CONFIG_SYS_MEMTEST_START	0x88000000
#define CONFIG_SYS_MEMTEST_END		0x88080000

/*----------------------------------------------------------------------
 * Commands
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_CMD_CLK

/*-------------------------------------------------
 * FLASH configuration
 */
#define CONFIG_SYS_NO_FLASH

/*------------------------------------------------------------
 * Console Configuration
 */
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_CBSIZE		1024 /* Console I/O Buffer Size   */
#define CONFIG_SYS_MAXARGS		16   /* max number of command args*/
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_CMDLINE_EDITING		1

/*
 * Handover flattened device tree (dtb file) to Linux kernel
 */
#define CONFIG_OF_LIBFDT	1

/* -------------------------------------------------
 * Environment
 */
#define CONFIG_ENV_IS_NOWHERE	1
#define CONFIG_ENV_SIZE		0x4000

/* ---------------------------------------------------------------------
 * Board boot configuration
 */
#define CONFIG_TIMESTAMP	/* Print image info with timestamp */
#define CONFIG_BOOTDELAY	5 /* autoboot after X seconds     */
#undef	CONFIG_BOOTARGS

#endif	/* __PIC32MZDASK_CONFIG_H */
