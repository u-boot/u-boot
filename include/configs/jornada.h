/*
 * Copyright 2010 (C)
 * Kristoffer Ericson <kristoffer.ericson@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SA1110			1	/* This is an SA110 CPU */
#define CONFIG_JORNADA700		1	/* on an HP Jornada 700 series */
#define CONFIG_SYS_FLASH_PROTECTION	1

#define CONFIG_SYS_TEXT_BASE		0xC1F00000

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_DCACHE_OFF

/* Console setting */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs      */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 * select serial console configuration
 */
#define CONFIG_SA1100_SERIAL	1
#define CONFIG_SERIAL3		1	/* we use serial 3 */
#define CONFIG_BAUDRATE	19200
#define CONFIG_LOADS_ECHO	1

/*
 * Command line configuration.
 */
#define CONFIG_CMD_JFFS2

#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOTARGS	"root=/dev/hda1 console=ttySA0,19200n8 console=tty1"
#define CONFIG_BOOTCOMMAND	"run boot_kernel"
#define CONFIG_SYS_AUTOLOAD	"n"	/* No autoload */
#define CONFIG_SYS_LOAD_ADDR	0xc0000000

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_PROMPT		"HP Jornada# "
#define CONFIG_SYS_CBSIZE		256	/* console buffsize */
#define CONFIG_SYS_PBSIZE		(256+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		256	/* Boot Argument Buffer Size */
#define CONFIG_SYS_MEMTEST_START	0xc0040000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0xc2000000	/* 4..128 MB */
#define CONFIG_SYS_CPUSPEED		0x0a /* core clock 206MHz */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 19200, 38400, 57600, 115200 }

#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER	1
#define CONFIG_FLASH_CFI_WIDTH		FLASH_CFI_32BIT
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_FLASH_ERASE_TOUT	(4096)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(4096)
#define CONFIG_SYS_FLASH_INCREMENT	0x02000000
#define PHYS_FLASH_1			0x00000000	/* starts at 0x0 */
#define PHYS_FLASH_SIZE		0x04000000	/* 64MB */
#define PHYS_FLASH_SECT_SIZE		0x00040000	/* 256KB Sectors */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	260
#define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1 }
#define CONFIG_SYS_FLASH_EMPTY_INFO	1
#define CONFIG_SYS_MONITOR_LEN		0x00040000
#define CONFIG_SYS_MONITOR_BASE	0x00000000
#define CONFIG_FLASH_SHOW_PROGRESS	1

/* Environment */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0x00040000
#define CONFIG_ENV_OFFSET	0x00040000
#define CONFIG_ENV_SIZE		0x00040000
#define CONFIG_ENV_SECT_SIZE	0x00040000
#define CONFIG_ENV_OVERWRITE	1

/*
  Monitor -     0x00000000 - 0x00040000 (256kb)
  Environment - 0x00040000 - 0x00080000 (256kb)
  Kernel -      0x00080000 - 0x00380000 (3mb)
  Rootfs -      0x00380000 - 0x........ (rest)
*/

#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_INTRAM_BASE		INTERNAL_SRAM_BASE
#define CONFIG_SYS_INTRAM_SIZE		INTERNAL_SRAM_SIZE
#define CONFIG_SYS_INIT_SP_ADDR	0x0
#define PHYS_SDRAM_1			0xc0000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_2			0xc4000000	/* SDRAM Bank #2 */
#define PHYS_SDRAM_1_SIZE		0x04000000	/* 64 MB */
#define PHYS_SDRAM_2_SIZE		0x04000000	/* 64 MB */

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=jornada7xx-0"
#define MTDPARTS_DEFAULT	"mtdparts=jornada7xx-0:256k(u-boot),256k(env),"\
		"3m(kernel),-(user);"

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"flash_kernel=protect off all; "				\
	"erase 00080000 0037ffff;cp.b c0000000 00080000 00300000;\0"	\
	"flash_uboot=protect off all; "					\
	"erase 00000000 0003ffff;cp.b c0000000 00000000 00040000;\0"	\
	"boot_kernel=cp.b 00080000 c0000000 00300000;bootm;\0"
#endif /* __CONFIG_H */
