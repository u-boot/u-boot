/*
 * Configuation settings for MPR2
 *
 * Copyright (C) 2008
 * Mark Jonas <mark.jonas@de.bosch.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MPR2_H
#define __MPR2_H

/* Supported commands */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_FLASH

/* Default environment variables */
#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTARGS		"console=ttySC0,115200"
#define CONFIG_BOOTFILE		/boot/zImage
#define CONFIG_LOADADDR		0x8E000000
#define CONFIG_VERSION_VARIABLE

/* CPU and platform */
#define CONFIG_SH		1
#define CONFIG_SH3		1
#define CONFIG_CPU_SH7720	1
#define CONFIG_MPR2		1

/* U-Boot internals */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt */
#define CFG_CBSIZE		256	/* Buffer size for input from the Console */
#define CFG_PBSIZE		256	/* Buffer size for Console output */
#define CFG_MAXARGS		16	/* max args accepted for monitor commands */
#define CFG_BARGSIZE		512	/* Buffer size for Boot Arguments passed to kernel */
#define CFG_BAUDRATE_TABLE	{ 115200 }	/* List of legal baudrate settings for this board */
#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 32 * 1024 * 1024)
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(128 * 1024)
#define CFG_MALLOC_LEN		(256 * 1024)
#define CFG_GBL_DATA_SIZE	256

/* Memory */
#define CFG_SDRAM_BASE		0x8C000000
#define CFG_SDRAM_SIZE		(64 * 1024 * 1024)
#define CFG_MEMTEST_START	CFG_SDRAM_BASE
#define CFG_MEMTEST_END		(CFG_MEMTEST_START + (60 * 1024 * 1024))

/* Flash */
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_BASE		0xA0000000
#define CFG_MAX_FLASH_SECT	256
#define CFG_MAX_FLASH_BANKS	1
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	(128 * 1024)
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_FLASH_ERASE_TOUT	120000
#define CFG_FLASH_WRITE_TOUT	500

/* Clocks */
#define CONFIG_SYS_CLK_FREQ	24000000
#define TMU_CLK_DIVIDER		4	/* 4 (default), 16, 64, 256 or 1024 */
#define CFG_HZ			(CONFIG_SYS_CLK_FREQ / TMU_CLK_DIVIDER)

/* UART */
#define CFG_SCIF_CONSOLE	1
#define CONFIG_CONS_SCIF0	1

#endif	/* __MPR2_H */
