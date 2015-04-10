/*
 * Sysam AMCORE board configuration
 *
 * (C) Copyright 2015  Angelo Dureghello <angelo@sysam.it>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __AMCORE_CONFIG_H
#define __AMCORE_CONFIG_H

#define CONFIG_AMCORE
#define CONFIG_HOSTNAME			AMCORE

#define CONFIG_MCFTMR
#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY		1
#define CONFIG_BOOTCOMMAND		"bootm ffc20000"

#include <config_cmd_default.h>
#undef CONFIG_CMD_AES
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_XIMG
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_TIMER
#define CONFIG_CMD_DIAG

#define CONFIG_SYS_PROMPT		"amcore $ "
/* undef to save memory	*/
#undef	CONFIG_SYS_LONGHELP

#if defined(CONFIG_CMD_KGDB)
/* Console I/O buff. size */
#define CONFIG_SYS_CBSIZE		1024
#else
#define CONFIG_SYS_CBSIZE		256
#endif
/* Print buffer size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args	*/
#define CONFIG_SYS_MAXARGS		16
/* Boot argument buffer size	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1 /* no console @ startup	*/
#define CONFIG_AUTO_COMPLETE		1 /* add autocompletion support	*/
#define CONFIG_LOOPW			1 /* enable loopw command	*/
#define CONFIG_MX_CYCLIC		1 /* enable mdc/mwc commands	*/

#define CONFIG_SYS_LOAD_ADDR		0x20000	/* default load address */

#define CONFIG_SYS_MEMTEST_START	0x0
#define CONFIG_SYS_MEMTEST_END		0x1000000

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_CLK			45000000
#define CONFIG_SYS_CPU_CLK		(CONFIG_SYS_CLK * 2)
/* Register Base Addrs */
#define CONFIG_SYS_MBAR			0x10000000
/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
/* size of internal SRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x1000000
#define CONFIG_SYS_FLASH_BASE		0xffc00000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	1024
#define CONFIG_SYS_FLASH_ERASE_TOUT	1000

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
/* amcore design has flash data bytes wired swapped */
#define CONFIG_SYS_WRITE_SWAPPED_DATA
/* reserve 128-4KB */
#define CONFIG_SYS_MONITOR_BASE		(CONFIG_SYS_FLASH_BASE + 0x400)
#define CONFIG_SYS_MONITOR_LEN          ((128 - 4) * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(64 * 1024)

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + \
					 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_SECT_SIZE		0x1000

/* memory map space for linux boot data */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Cache Configuration
 *
 * Special 8K version 3 core cache.
 * This is a single unified instruction/data cache.
 * sdram - single region - no masks
 */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV           (CF_CACR_CINVA)
#define CONFIG_SYS_CACHE_ACR0		(CF_ACR_CM_WT | CF_ACR_SM_ALL | \
					 CF_ACR_EN)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_DCM_P | CF_CACR_ESB | \
					 CF_CACR_EC)

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CONFIG_SYS_CS0_BASE		(CONFIG_SYS_FLASH_BASE>>16)
/* 4MB, AA=0,V=1  C/I BIT for errata */
#define	CONFIG_SYS_CS0_MASK		0x003f0001
/* WS=10, AA=1, PS=16bit (10) */
#define	CONFIG_SYS_CS0_CTRL		0x1980
/* CS1 - DM9000 Ethernet Controller, address 0x30000000 */
#define CONFIG_SYS_CS1_BASE		0x3000
#define CONFIG_SYS_CS1_MASK		0x00070001
#define CONFIG_SYS_CS1_CTRL		0x0100

#endif  /* __AMCORE_CONFIG_H */

