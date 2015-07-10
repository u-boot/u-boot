/*
 * (C) Copyright 2007
 * Stefano Babic, DENX Gmbh, sbabic@denx.de
 *
 * (C) Copyright 2004
 * Robert Whaley, Applied Data Systems, Inc. rwhaley@applieddata.net
 *
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the LUBBOCK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_CPU_PXA27X		1	/* This is an PXA27x CPU    */

#define CONFIG_MMC		1
#define CONFIG_BOARD_LATE_INIT
#define	CONFIG_SYS_TEXT_BASE	0x0

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_DCACHE_OFF

#define RTC

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART	       1       /* we use FFUART on Conxs */
#define CONFIG_BTUART	       1       /* we use BTUART on Conxs */
#define CONFIG_STUART	       1       /* we use STUART on Conxs */
#define CONFIG_CONS_INDEX	3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE	       38400

#define CONFIG_DOS_PARTITION   1

/*
 * Command line configuration.
 */
#define CONFIG_CMD_FAT
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */

#undef CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_BOOTDELAY	3
#define CONFIG_SERVERIP		192.168.1.99
#define CONFIG_BOOTCOMMAND	"run boot_flash"
#define CONFIG_BOOTARGS		"console=ttyS0,38400 ramdisk_size=12288"\
				" rw root=/dev/ram initrd=0xa0800000,5m"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"program_boot_mmc="						\
			"mw.b 0xa0010000 0xff 0x20000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0010000 u-boot.bin; "	\
			"then "						\
				"protect off 0x0 0x1ffff; "		\
				"erase 0x0 0x1ffff; "			\
				"cp.b 0xa0010000 0x0 0x20000; "		\
			"fi\0"						\
	"program_uzImage_mmc="						\
			"mw.b 0xa0010000 0xff 0x180000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0010000 uzImage; "	\
			"then "						\
				"protect off 0x40000 0x1bffff; "	\
				"erase 0x40000 0x1bffff; "		\
				"cp.b 0xa0010000 0x40000 0x180000; "	\
			"fi\0"						\
	"program_ramdisk_mmc="						\
			"mw.b 0xa0010000 0xff 0x500000; "		\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0010000 ramdisk.gz; "	\
			"then "						\
				"protect off 0x1c0000 0x6bffff; "	\
				"erase 0x1c0000 0x6bffff; "		\
				"cp.b 0xa0010000 0x1c0000 0x500000; "	\
			"fi\0"						\
	"boot_mmc="							\
			"if	 mmcinit && "				\
				"fatload mmc 0 0xa0030000 uzImage && "	\
				"fatload mmc 0 0xa0800000 ramdisk.gz; "	\
			"then "						\
				"bootm 0xa0030000; "			\
			"fi\0"						\
	"boot_flash="							\
			"cp.b 0x1c0000 0xa0800000 0x500000; "		\
			"bootm 0x40000\0"				\

#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
/* #define CONFIG_INITRD_TAG	 1 */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER		1

#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#endif
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_DEVICE_NULLDEV	1

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0xa1000000	/* default load address */

#define CONFIG_SYS_CPUSPEED		0x207		/* need to look more closely, I think this is Turbo = 2x, L=91Mhz */

#ifdef CONFIG_MMC
#define	CONFIG_GENERIC_MMC
#define	CONFIG_PXA_MMC_GENERIC
#define CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_BASE		0xF0000000
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */
#define PHYS_SDRAM_2		0xa4000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_3		0xa8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_4		0xac000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE	0x00000000 /* 0 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CONFIG_SYS_DRAM_BASE		0xa0000000
#define CONFIG_SYS_DRAM_SIZE		0x04000000

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		(GENERATED_GBL_DATA_SIZE + PHYS_SDRAM_1)

/*
 * GPIO settings
 */
#define CONFIG_SYS_GPSR0_VAL		0x00018000
#define CONFIG_SYS_GPSR1_VAL		0x00000000
#define CONFIG_SYS_GPSR2_VAL		0x400dc000
#define CONFIG_SYS_GPSR3_VAL		0x00000000
#define CONFIG_SYS_GPCR0_VAL		0x00000000
#define CONFIG_SYS_GPCR1_VAL		0x00000000
#define CONFIG_SYS_GPCR2_VAL		0x00000000
#define CONFIG_SYS_GPCR3_VAL		0x00000000
#define CONFIG_SYS_GPDR0_VAL		0x00018000
#define CONFIG_SYS_GPDR1_VAL		0x00028801
#define CONFIG_SYS_GPDR2_VAL		0x520dc000
#define CONFIG_SYS_GPDR3_VAL		0x0001E000
#define CONFIG_SYS_GAFR0_L_VAL		0x801c0000
#define CONFIG_SYS_GAFR0_U_VAL		0x00000013
#define CONFIG_SYS_GAFR1_L_VAL		0x6990100A
#define CONFIG_SYS_GAFR1_U_VAL		0x00000008
#define CONFIG_SYS_GAFR2_L_VAL		0xA0000000
#define CONFIG_SYS_GAFR2_U_VAL		0x010900F2
#define CONFIG_SYS_GAFR3_L_VAL		0x54000003
#define CONFIG_SYS_GAFR3_U_VAL		0x00002401
#define CONFIG_SYS_GRER0_VAL		0x00000000
#define CONFIG_SYS_GRER1_VAL		0x00000000
#define CONFIG_SYS_GRER2_VAL		0x00000000
#define CONFIG_SYS_GRER3_VAL		0x00000000

#define CONFIG_SYS_GFER1_VAL		0x00000000
#define CONFIG_SYS_GFER3_VAL		0x00000020

#if CONFIG_POLARIS
#define CONFIG_SYS_GFER0_VAL		0x00000001
#define CONFIG_SYS_GFER2_VAL		0x00200000
#else
#define CONFIG_SYS_GFER0_VAL		0x00000000
#define CONFIG_SYS_GFER2_VAL		0x00000000
#endif

#define CONFIG_SYS_PSSR_VAL		0x20	/* CHECK */

/*
 * Clock settings
 */
#define CONFIG_SYS_CKEN		0x01FFFFFF	/* CHECK */
#define CONFIG_SYS_CCCR		0x02000290 /*   520Mhz */

/*
 * Memory settings
 */

#define CONFIG_SYS_MSC0_VAL		0x4df84df0
#define CONFIG_SYS_MSC1_VAL		0x7ff87ff4
#if CONFIG_POLARIS
#define CONFIG_SYS_MSC2_VAL		0xa2697ff8
#else
#define CONFIG_SYS_MSC2_VAL		0xa26936d4
#endif
#define CONFIG_SYS_MDCNFG_VAL		0x880009C9
#define CONFIG_SYS_MDREFR_VAL		0x20ca201e
#define CONFIG_SYS_MDMRS_VAL		0x00220022

#define CONFIG_SYS_FLYCNFG_VAL		0x00000000
#define CONFIG_SYS_SXCNFG_VAL		0x40044004

/*
 * PCMCIA and CF Interfaces
 */
#define CONFIG_SYS_MECR_VAL		0x00000001
#define CONFIG_SYS_MCMEM0_VAL		0x00004204
#define CONFIG_SYS_MCMEM1_VAL		0x00010204
#define CONFIG_SYS_MCATT0_VAL		0x00010504
#define CONFIG_SYS_MCATT1_VAL		0x00010504
#define CONFIG_SYS_MCIO0_VAL		0x00008407
#define CONFIG_SYS_MCIO1_VAL		0x0000c108

#define CONFIG_DRIVER_DM9000		1

#if CONFIG_POLARIS
#define CONFIG_DM9000_BASE		0x0C800000
#else
#define CONFIG_DM9000_BASE		0x08000000
#endif

#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE+0x8004)

#define CONFIG_USB_OHCI_NEW	1
#define CONFIG_SYS_USB_OHCI_BOARD_INIT	1
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	3
#define CONFIG_SYS_USB_OHCI_REGS_BASE	0x4C000000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"trizepsiv"
#define CONFIG_USB_STORAGE	1
#define CONFIG_SYS_USB_OHCI_CPU_INIT	1

/*
 * FLASH and environment organization
 */

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER	1

#define CONFIG_SYS_MONITOR_BASE	0
#define CONFIG_SYS_MONITOR_LEN		0x40000

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	4 + 255  /* max number of sectors on one chip   */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* write flash less slowly */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1

/* Unlock to be used with Intel chips */
#define CONFIG_SYS_FLASH_PROTECTION	1

/* Flash environment locations */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + CONFIG_SYS_MONITOR_LEN) /* Addr of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x40000	/* Total Size of Environment		*/
#define CONFIG_ENV_SECT_SIZE	0x40000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#endif	/* __CONFIG_H */
