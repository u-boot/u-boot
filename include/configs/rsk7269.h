/*
 * Configuation settings for the Renesas RSK2+SH7269 board
 *
 * Copyright (C) 2012 Renesas Electronics Europe Ltd.
 * Copyright (C) 2012 Phil Edworthy
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RSK7269_H
#define __RSK7269_H

#undef DEBUG
#define CONFIG_CPU_SH7269	1
#define CONFIG_RSK7269		1

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTARGS		"console=ttySC7,115200"
#define CONFIG_BOOTDELAY	3
#define CONFIG_SYS_BAUDRATE_TABLE	{ CONFIG_BAUDRATE }

#define CONFIG_SYS_LONGHELP		/* undef to save memory	*/
#define CONFIG_SYS_CBSIZE	256	/* Boot Argument Buffer Size */
#define CONFIG_SYS_PBSIZE	256	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */

/* Serial */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_CONS_SCIF7

/* Memory */
/* u-boot relocated to top 256KB of ram */
#define CONFIG_SYS_TEXT_BASE		0x0DFC0000
#define CONFIG_SYS_SDRAM_BASE		0x0C000000
#define CONFIG_SYS_SDRAM_SIZE		(32 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - 0x100000)
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 4*1024*1024)

/* NOR Flash */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_BASE		0x20000000 /* Non-cached */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	512

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET	(128 * 1024)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	66125000
#define CONFIG_SH_TMU_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CMT_CLK_DIVIDER		32	/* 8 (default), 32, 128 or 512 */
#define CONFIG_SH_CMT_CLK_FREQ (CONFIG_SYS_CLK_FREQ / CMT_CLK_DIVIDER)

/* Network interface */
#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE	0x24000000

#endif	/* __RSK7269_H */
