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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/pxa-regs.h>

/*
 * If we are developing, we might want to start U-Boot from RAM
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#undef CONFIG_SKIP_LOWLEVEL_INIT			/* define for developing */
#define	CONFIG_SYS_TEXT_BASE	0x0

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
#define CONFIG_CPU_PXA25X		1	/* This is an PXA250 CPU    */

#undef CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_PXA_LCD
#define CONFIG_SHARP_LM8V31
#endif

#define CONFIG_MMC		1
#define CONFIG_DOS_PARTITION	1
#define CONFIG_BOARD_LATE_INIT

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_DCACHE_OFF

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)

/*
 * PXA250 IDP memory map information
 */

#define IDP_CS5_ETH_OFFSET	0x03400000


/*
 * Hardware drivers
 */
#define CONFIG_SMC91111
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
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART	       1       /* we use FFUART on LUBBOCK */
#define CONFIG_CONS_INDEX	3

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
#define CONFIG_SYS_HUSH_PARSER		1

#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt */
#endif
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_DEVICE_NULLDEV	1

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0xa0800000	/* default load address */

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_CPUSPEED		0x161		/* set core clock to 400/200/100 MHz */

#define RTC	1				/* enable 32KHz osc */

#ifdef CONFIG_MMC
#define	CONFIG_GENERIC_MMC
#define	CONFIG_PXA_MMC_GENERIC
#define CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_BASE		0xF0000000
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
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

#define CONFIG_SYS_DRAM_BASE		0xa0000000
#define CONFIG_SYS_DRAM_SIZE		0x04000000

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		0xfffff800

/*
 * GPIO settings
 */

#define CONFIG_SYS_GAFR0_L_VAL	0x80001005
#define CONFIG_SYS_GAFR0_U_VAL	0xa5128012
#define CONFIG_SYS_GAFR1_L_VAL	0x699a9558
#define CONFIG_SYS_GAFR1_U_VAL	0xaaa5aa6a
#define CONFIG_SYS_GAFR2_L_VAL	0xaaaaaaaa
#define CONFIG_SYS_GAFR2_U_VAL	0x2
#define CONFIG_SYS_GPCR0_VAL	0x1800400
#define CONFIG_SYS_GPCR1_VAL	0x0
#define CONFIG_SYS_GPCR2_VAL	0x0
#define CONFIG_SYS_GPDR0_VAL	0xc1818440
#define CONFIG_SYS_GPDR1_VAL	0xfcffab82
#define CONFIG_SYS_GPDR2_VAL	0x1ffff
#define CONFIG_SYS_GPSR0_VAL	0x8000
#define CONFIG_SYS_GPSR1_VAL	0x3f0002
#define CONFIG_SYS_GPSR2_VAL	0x1c000

#define CONFIG_SYS_PSSR_VAL		0x20

#define	CONFIG_SYS_CCCR			CCCR_L27|CCCR_M2|CCCR_N10
#define	CONFIG_SYS_CKEN			0x0

/*
 * Memory settings
 */
#define CONFIG_SYS_MSC0_VAL		0x29DCA4D2
#define CONFIG_SYS_MSC1_VAL		0x43AC494C
#define CONFIG_SYS_MSC2_VAL		0x39D449D4
#define CONFIG_SYS_MDCNFG_VAL		0x090009C9
#define CONFIG_SYS_MDREFR_VAL		0x0085C017
#define CONFIG_SYS_MDMRS_VAL		0x00220022
#define	CONFIG_SYS_FLYCNFG_VAL		0x00000000
#define	CONFIG_SYS_SXCNFG_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CONFIG_SYS_MECR_VAL		0x00000003
#define CONFIG_SYS_MCMEM0_VAL		0x00014405
#define CONFIG_SYS_MCMEM1_VAL		0x00014405
#define CONFIG_SYS_MCATT0_VAL		0x00014405
#define CONFIG_SYS_MCATT1_VAL		0x00014405
#define CONFIG_SYS_MCIO0_VAL		0x00014405
#define CONFIG_SYS_MCIO1_VAL		0x00014405

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER	1

#define CONFIG_SYS_MONITOR_BASE	0
#define CONFIG_SYS_MONITOR_LEN		PHYS_FLASH_SECT_SIZE

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* put cfg at end of flash for now */
#define CONFIG_ENV_IS_IN_FLASH	1
 /* Addr of Environment Sector	*/
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + PHYS_FLASH_SIZE - 0x40000)
#define CONFIG_ENV_SIZE		PHYS_FLASH_SECT_SIZE	/* Total Size of Environment Sector	*/
#define	CONFIG_ENV_SECT_SIZE	(PHYS_FLASH_SECT_SIZE / 16)

#endif	/* __CONFIG_H */
