/*
 * (C) Copyright 2005
 * Greg Ungerer <greg.ungerer@opengear.com>.
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_KS8695	1		/* it is a KS8695 CPU */
#define CONFIG_CM41xx	1		/* it is an OpenGear CM41xx boad */

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

#define CONFIG_DRIVER_KS8695ETH		/* use KS8695 ethernet driver	*/

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_KS8695_SERIAL
#define	CONFIG_SERIAL1
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_SAVEENV


#define CONFIG_BOOTDELAY	0
#define CONFIG_BOOTARGS		"mem=32M console=ttyAM0,115200"
#define CONFIG_BOOTCOMMAND	"gofsk 0x02200000"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"boot > "	/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00800000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x01000000	/* 16 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00008000	/* default load address */

#define CONFIG_SYS_HZ			(1000)		/* 1ms resolution ticks */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_SYS_INIT_SP_ADDR	0x00020000 /* lowest 128k of RAM */

#define PHYS_FLASH_1		0x02000000 /* Flash Bank #1 */
#define PHYS_FLASH_SECT_SIZE    0x00020000 /* 128 KB sectors (x1) */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of flash banks */
#define CONFIG_SYS_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(20*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(20*CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define CONFIG_ENV_SIZE		0x20000     /* Total Size of Environment */

#endif	/* __CONFIG_H */
