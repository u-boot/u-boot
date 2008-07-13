/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copied from lubbock.h
 *
 * (C) Copyright 2004
 * BEC Systems <http://bec-systems.com>
 * Cliff Brake <cliff.brake@gmail.com>
 * Configuation settings for the Accelent/Vibren PXA255 IDP
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/pxa-regs.h>

/*
 * If we are developing, we might want to start U-Boot from RAM
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#undef CONFIG_SKIP_LOWLEVEL_INIT			/* define for developing */
#undef CONFIG_SKIP_RELOCATE_UBOOT			/* define for developing */

/*
 * define the following to enable debug blinks.  A debug blink function
 * must be defined in memsetup.S
 */
#undef DEBUG_BLINK_ENABLE
#undef DEBUG_BLINKC_ENABLE

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_PXA250		1	/* This is an PXA250 CPU    */

#undef CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_SHARP_LM8V31
#endif

#define CONFIG_MMC		1
#define CONFIG_DOS_PARTITION	1
#define BOARD_LATE_INIT		1

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	    (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * PXA250 IDP memory map information
 */

#define IDP_CS5_ETH_OFFSET	0x03400000


/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE	(PXA_CS5_PHYS + IDP_CS5_ETH_OFFSET + 0x300)
#define CONFIG_SMC_USE_32_BIT	1
/* #define CONFIG_SMC_USE_IOFUNCS */

/* the following has to be set high -- suspect something is wrong with
 * with the tftp timeout routines. FIXME!!!
 */
#define CONFIG_NET_RETRY_COUNT	100

/*
 * select serial console configuration
 */
#define CONFIG_FFUART	       1       /* we use FFUART on LUBBOCK */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

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

#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_DHCP

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"bootm 40000"
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2 rootfstype=cramfs console=ttyS0,115200"

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
/* #define CONFIG_INITRD_TAG		1 */

/*
 * Current memory map for Vibren supplied Linux images:
 *
 * Flash:
 * 0 - 0x3ffff (size = 0x40000): bootloader
 * 0x40000 - 0x13ffff (size = 0x100000): kernel
 * 0x140000 - 0x1f3ffff (size = 0x1e00000): jffs
 *
 * RAM:
 * 0xa0008000 - kernel is loaded
 * 0xa3000000 - Uboot runs (48MB into RAM)
 *
 */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"prog_boot_mmc="						\
			"mw.b 0xa0000000 0xff 0x40000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0000000 u-boot.bin; "	\
			"then "						\
				"protect off 0x0 0x3ffff; "		\
				"erase 0x0 0x3ffff; "			\
				"cp.b 0xa0000000 0x0 0x40000; "		\
				"reset;"				\
			"fi\0"						\
	"prog_uzImage_mmc="						\
			"mw.b 0xa0000000 0xff 0x100000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0000000 uzImage; "	\
			"then "						\
				"protect off 0x40000 0xfffff; "		\
				"erase 0x40000 0xfffff; "		\
				"cp.b 0xa0000000 0x40000 0x100000; "	\
			"fi\0"						\
	"prog_jffs_mmc="						\
			"mw.b 0xa0000000 0xff 0x1e00000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0000000 root.jffs; "	\
			"then "						\
				"protect off 0x140000 0x1f3ffff; "	\
				"erase 0x140000 0x1f3ffff; "		\
				"cp.b 0xa0000000 0x140000 0x1e00000; "	\
			"fi\0"						\
	"boot_mmc="							\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa1000000 uzImage && "	\
			"then "						\
				"bootm 0xa1000000; "			\
			"fi\0"						\
	"prog_boot_net="						\
			"mw.b 0xa0000000 0xff 0x100000; "		\
			"if	 bootp 0xa0000000 u-boot.bin; "		\
			"then "						\
				"protect off 0x0 0x3ffff; "		\
				"erase 0x0 0x3ffff; "			\
				"cp.b 0xa0000000 0x0 0x40000; "		\
				"reset; "				\
			"fi\0"						\
	"prog_uzImage_net="						\
			"mw.b 0xa0000000 0xff 0x100000; "		\
			"if	 bootp 0xa0000000 uzImage; "		\
			"then "						\
				"protect off 0x40000 0xfffff; "		\
				"erase 0x40000 0xfffff; "		\
				"cp.b 0xa0000000 0x40000 0x100000; "	\
			"fi\0"						\
	"prog_jffs_net="						\
			"mw.b 0xa0000000 0xff 0x1e00000; "		\
			"if	 bootp 0xa0000000 root.jffs; "		\
			"then "						\
				"protect off 0x140000 0x1f3ffff; "	\
				"erase 0x140000 0x1f3ffff; "		\
				"cp.b 0xa0000000 0x140000 0x1e00000; "	\
			"fi\0"


/*	"erase_env="			*/
/*			"protect off"	*/


#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP				/* undef to save memory		*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */
#endif
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_DEVICE_NULLDEV	1

#define CFG_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0xa0800000	/* default load address */

#define CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x161		/* set core clock to 400/200/100 MHz */

#define RTC	1				/* enable 32KHz osc */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_MMC_BASE		0xF0000000

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 1 banks of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */
#define PHYS_SDRAM_2		0xa4000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_3		0xa8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_4		0xac000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE	0x00000000 /* 0 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x02000000 /* 32 MB */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00040000 /* 256 KB sectors (x2) */

#define CFG_DRAM_BASE		0xa0000000
#define CFG_DRAM_SIZE		0x04000000

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*
 * GPIO settings
 */

#define CFG_GAFR0_L_VAL	0x80001005
#define CFG_GAFR0_U_VAL	0xa5128012
#define CFG_GAFR1_L_VAL	0x699a9558
#define CFG_GAFR1_U_VAL	0xaaa5aa6a
#define CFG_GAFR2_L_VAL	0xaaaaaaaa
#define CFG_GAFR2_U_VAL	0x2
#define CFG_GPCR0_VAL	0x1800400
#define CFG_GPCR1_VAL	0x0
#define CFG_GPCR2_VAL	0x0
#define CFG_GPDR0_VAL	0xc1818440
#define CFG_GPDR1_VAL	0xfcffab82
#define CFG_GPDR2_VAL	0x1ffff
#define CFG_GPSR0_VAL	0x8000
#define CFG_GPSR1_VAL	0x3f0002
#define CFG_GPSR2_VAL	0x1c000

#define CFG_PSSR_VAL		0x20

/*
 * Memory settings
 */
#define CFG_MSC0_VAL		0x29DCA4D2
#define CFG_MSC1_VAL		0x43AC494C
#define CFG_MSC2_VAL		0x39D449D4
#define CFG_MDCNFG_VAL		0x090009C9
#define CFG_MDREFR_VAL		0x0085C017
#define CFG_MDMRS_VAL		0x00220022

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL		0x00000003
#define CFG_MCMEM0_VAL		0x00014405
#define CFG_MCMEM1_VAL		0x00014405
#define CFG_MCATT0_VAL		0x00014405
#define CFG_MCATT1_VAL		0x00014405
#define CFG_MCIO0_VAL		0x00014405
#define CFG_MCIO1_VAL		0x00014405

/*
 * FLASH and environment organization
 */
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER	1

#define CFG_MONITOR_BASE	0
#define CFG_MONITOR_LEN		PHYS_FLASH_SECT_SIZE

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

#define CFG_FLASH_USE_BUFFER_WRITE	1

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(25*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(25*CFG_HZ) /* Timeout for Flash Write */

/* put cfg at end of flash for now */
#define CFG_ENV_IS_IN_FLASH	1
 /* Addr of Environment Sector	*/
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + PHYS_FLASH_SIZE - 0x40000)
#define CFG_ENV_SIZE		PHYS_FLASH_SECT_SIZE	/* Total Size of Environment Sector	*/
#define	CFG_ENV_SECT_SIZE	(PHYS_FLASH_SECT_SIZE / 16)

#endif	/* __CONFIG_H */
