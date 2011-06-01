/*
 * Configuation settings for the Renesas Technology RSK 7264
 *
 * Copyright (C) 2011 Renesas Electronics Europe Ltd.
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 * Copyright (C) 2008 Renesas Solutions Corp.
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef __RSK7264_H
#define __RSK7264_H

#undef DEBUG
#define CONFIG_SH		1
#define CONFIG_SH2		1
#define CONFIG_SH2A		1
#define CONFIG_CPU_SH7264	1
#define CONFIG_RSK7264	1

#define CONFIG_CMD_FLASH
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_CACHE

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTARGS		"console=ttySC3,115200"
#define CONFIG_BOOTDELAY	3
#define CONFIG_LOADADDR		0x0C100000 /* RSK7264_SDRAM_BASE + 1MB */

#define CONFIG_VERSION_VARIABLE
#undef	CONFIG_SHOW_BOOT_PROGRESS

/* MEMORY */
#define RSK7264_SDRAM_BASE	0x0C000000
#define RSK7264_FLASH_BASE_1	0x20000000	/* Non cache */

#define CONFIG_SYS_TEXT_BASE	0x0C1C0000
#define CONFIG_SYS_LONGHELP		/* undef to save memory	*/
#define CONFIG_SYS_PROMPT	"=> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256	/* Buffer size for input from the Console */
#define CONFIG_SYS_PBSIZE	256	/* Buffer size for Console output */
#define CONFIG_SYS_MAXARGS	16	/* max args accepted for monitor commands */
/* Buffer size for Boot Arguments passed to kernel */
#define CONFIG_SYS_BARGSIZE	512
/* List of legal baudrate settings for this board */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE	1
#define CONFIG_CONS_SCIF3	1

#define CONFIG_SYS_MEMTEST_START	RSK7264_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (3 * 1024 * 1024))

#define CONFIG_SYS_SDRAM_BASE		RSK7264_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 1024 * 1024)
#define CONFIG_SYS_MONITOR_BASE		RSK7264_FLASH_BASE_1
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/* FLASH */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_WIDTH 	FLASH_CFI_16BIT
#undef	CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_BASE		RSK7264_FLASH_BASE_1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	33333333
#define CMT_CLK_DIVIDER	32	/* 8 (default), 32, 128 or 512 */
#define CONFIG_SYS_HZ		(CONFIG_SYS_CLK_FREQ / CMT_CLK_DIVIDER)

/* Network interface */
#define CONFIG_NET_MULTI
#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE (0x28000000)

#endif	/* __RSK7264_H */
