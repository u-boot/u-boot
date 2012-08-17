/*
 * Copyright (C) 2011 Ilya Yanok, Emcraft Systems
 *
 * Based on omap3_evm_config.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP			/* in a TI OMAP core */
#define CONFIG_OMAP34XX			/* which is a 34XX */
#define CONFIG_OMAP3_MCX		/* working with mcx */

#define MACH_TYPE_MCX			3656
#define CONFIG_MACH_TYPE	MACH_TYPE_MCX

#define CONFIG_SYS_CACHELINE_SIZE	64

#define CONFIG_EMIF4	/* The chip has EMIF4 controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

#define CONFIG_OF_LIBFDT
#define CONFIG_FIT

/*
 * Leave it at 0x80008000 to allow booting new u-boot.bin with X-loader
 * and older u-boot.bin with the new U-Boot SPL.
 */
#define CONFIG_SYS_TEXT_BASE		0x80008000

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB sector */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)
/*
 * DDR related
 */
#define CONFIG_SYS_CS0_SIZE		(256 * 1024 * 1024)

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3	/* UART3 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION

/* EHCI */
#define CONFIG_USB_STORAGE
#define CONFIG_OMAP3_GPIO_5
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT_OMAP
/*#define CONFIG_EHCI_DCACHE*/ /* leave it disabled for now */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	154
#define CONFIG_OMAP_EHCI_PHY2_RESET_GPIO	152
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 3

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/

#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_USB
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_DRIVER_OMAP34XX_I2C

/* RTC */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access */
							/* nand at CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */
#define CONFIG_SYS_64BIT_VSPRINTF		/* needed for nand_util.c */

#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* sz of jffs2 part */

/* Environment information */
#define CONFIG_BOOTDELAY	10

#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=/dev/mtdblock4 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"if mmc init; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/*
 * Miscellaneous configurable options
 */
#define V_PROMPT			"mcx # "

#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		V_PROMPT
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command */
						/* args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * AM3517 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#define CONFIG_NAND_OMAP_GPMC
#define GPMC_NAND_ECC_LP_x16_LAYOUT
#define CONFIG_ENV_IS_IN_NAND
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

/*
 * CFI FLASH driver setup
 */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100 * CONFIG_SYS_HZ)

/* Flash banks JFFS2 should use */
#define CONFIG_SYS_MAX_MTD_BANKS	(CONFIG_SYS_MAX_FLASH_BANKS + \
					CONFIG_SYS_MAX_NAND_DEVICE)
#define CONFIG_SYS_JFFS2_MEM_NAND
/* use flash_info[2] */
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_SOFTECC

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

#define CONFIG_SPL_TEXT_BASE		0x40200000 /*CONFIG_SYS_SRAM_START*/
#define CONFIG_SPL_MAX_SIZE		(45 << 10)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

/* move malloc and bss high to prevent clashing with the main image */
#define CONFIG_SYS_SPL_MALLOC_START	0x8f000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x8f080000 /* end of RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

/* NAND boot config */
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{40, 41, 42, 43, 44, 45, 46, 47,\
					 48, 49, 50, 51, 52, 53, 54, 55,\
					 56, 57, 58, 59, 60, 61, 62, 63}
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3

#define CONFIG_SYS_NAND_U_BOOT_START   CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * ethernet support
 *
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT 10
#endif

#endif /* __CONFIG_H */
