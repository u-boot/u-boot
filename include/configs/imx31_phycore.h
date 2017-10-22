/*
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the phyCORE-i.MX31 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */
#define CONFIG_MX31			/* This is a mx31 */
#define CONFIG_MX31_CLK32	32000

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 512 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_CLK_OFFSET	I2C2_CLK_OFFSET

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1

/***********************************************************
 * Command definition
 ***********************************************************/

#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.23.168
#define CONFIG_SERVERIP		192.168.23.2

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"bootargs_base=setenv bootargs console=ttySMX0,115200\0"	\
	"bootargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs "	\
		"ip=dhcp nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0"	\
	"bootargs_flash=setenv bootargs $(bootargs) "			\
		"root=/dev/mtdblock2 rootfstype=jffs2\0"		\
	"bootargs_mtd=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"bootcmd=run bootcmd_net\0"					\
	"bootcmd_net=run bootargs_base bootargs_mtd bootargs_nfs;"	\
		"tftpboot 0x80000000 $(uimage);bootm\0"			\
	"bootcmd_flash=run bootargs_base bootargs_mtd bootargs_flash;"	\
		"bootm 0x80000000\0"					\
	"unlock=yes\0"							\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"				\
	"prg_uboot=tftpboot 0x80000000 $(uboot);"			\
		"protect off 0xa0000000 +0x20000;"			\
		"erase 0xa0000000 +0x20000;"				\
		"cp.b 0x80000000 0xa0000000 $(filesize)\0"		\
	"prg_kernel=tftpboot 0x80000000 $(uimage);"			\
		"erase 0xa0040000 +0x180000;"				\
		"cp.b 0x80000000 0xa0040000 $(filesize)\0"		\
	"prg_jffs2=tftpboot 0x80000000 $(jffs2);"			\
		"erase 0xa01c0000 0xa1ffffff;"				\
		"cp.b 0x80000000 0xa01c0000 $(filesize)\0"		\
	"videomode=video=ctfb:x:240,y:320,depth:16,mode:0,"		\
		"pclk:185925,le:9,ri:17,up:7,lo:10,hs:1,vs:1,"		\
		"sync:1241513985,vmode:0\0"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#define CONFIG_SYS_MEMTEST_START	0  /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#define CONFIG_SYS_LOAD_ADDR		0 /* default load address */

#define CONFIG_CMDLINE_EDITING

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		(128 * 1024 * 1024)
#define CONFIG_SYS_TEXT_BASE		0xA0000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_GBL_DATA_OFFSET)

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		0xa0000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max # of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	259	/* max # of sectors/chip */
/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_ENV_OFFSET			0x00	/* env. starts here */
#define CONFIG_ENV_SIZE				4096
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x52
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* 5 bits = 32 octets */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* 10 ms delay */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2	/* byte addr. lenght */

/*
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI		/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER		/* Use drivers/mtd/cfi_flash.c */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_PROTECTION	/* Use hardware sector protection */

/*
 * Timeout for Flash Erase and Flash Write
 * timeout values are in ticks
 */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100*CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100*CONFIG_SYS_HZ)

/*
 * JFFS2 partitions
 */
#define CONFIG_JFFS2_DEV	"nor0"

/* EET platform additions */
#ifdef CONFIG_TARGET_IMX31_PHYCORE_EET
#define CONFIG_MXC_GPIO

#define CONFIG_HARD_SPI
#define CONFIG_MXC_SPI

#define CONFIG_S6E63D6

#define CONFIG_VIDEO_MX3
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_BMP_16BPP
#endif

#endif /* __CONFIG_H */
