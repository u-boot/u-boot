/*
 * Palm Tungsten|C configuration file
 *
 * Copyright (C) 2009-2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef	__CONFIG_H
#define	__CONFIG_H

#include <asm/arch/pxa-regs.h>

/*
 * High Level Board Configuration Options
 */
#define	CONFIG_CPU_PXA25X			1	/* Intel PXA255 CPU */
#define	CONFIG_PALMTC			1	/* Palm Tungsten|C board */

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128*1024)
#define	CONFIG_SYS_TEXT_BASE	0x0

#define	CONFIG_BOOTCOMMAND						\
	"if mmc init && fatload mmc 0 0xa0000000 uboot.script ; then "	\
		"source 0xa0000000; "					\
	"else "								\
		"bootm 0x80000; "					\
	"fi; "
#define	CONFIG_BOOTARGS							\
	"console=tty0 console=ttyS0,115200"
#define	CONFIG_TIMESTAMP
#define	CONFIG_BOOTDELAY		2	/* Autoboot delay */
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS

#define	CONFIG_LZMA			/* LZMA compression support */

/*
 * Serial Console Configuration
 * STUART - the lower serial port on Colibri board
 */
#define	CONFIG_PXA_SERIAL
#define	CONFIG_FFUART			1
#define	CONFIG_BAUDRATE			115200
#define	CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Bootloader Components Configuration
 */
#include <config_cmd_default.h>

#undef	CONFIG_CMD_NET
#undef	CONFIG_CMD_NFS
#define	CONFIG_CMD_ENV
#define	CONFIG_CMD_MMC
#define	CONFIG_LCD

/*
 * MMC Card Configuration
 */
#ifdef	CONFIG_CMD_MMC
#define	CONFIG_MMC
#define	CONFIG_PXA_MMC
#define	CONFIG_SYS_MMC_BASE		0xF0000000
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_EXT2
#define	CONFIG_DOS_PARTITION
#endif

/*
 * LCD
 */
#ifdef	CONFIG_LCD
#define	CONFIG_ACX517AKN
#define	CONFIG_VIDEO_LOGO
#define	CONFIG_CMD_BMP
#define	CONFIG_SPLASH_SCREEN
#define	CONFIG_SPLASH_SCREEN_ALIGN
#define	CONFIG_VIDEO_BMP_GZIP
#define	CONFIG_VIDEO_BMP_RLE8
#define	CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)
#endif

/*
 * KGDB
 */
#ifdef	CONFIG_CMD_KGDB
#define	CONFIG_KGDB_BAUDRATE		230400	/* kgdb serial port speed */
#define	CONFIG_KGDB_SER_INDEX		2	/* which serial port to use */
#endif

/*
 * HUSH Shell Configuration
 */
#define	CONFIG_SYS_HUSH_PARSER		1
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define	CONFIG_SYS_LONGHELP
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT		"$ "
#else
#define	CONFIG_SYS_PROMPT		"=> "
#endif
#define	CONFIG_SYS_CBSIZE		256
#define	CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define	CONFIG_SYS_MAXARGS		16
#define	CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define	CONFIG_SYS_DEVICE_NULLDEV	1

/*
 * Clock Configuration
 */
#undef	CONFIG_SYS_CLKS_IN_HZ
#define	CONFIG_SYS_HZ			3686400		/* Timer @ 3686400 Hz */
#define	CONFIG_SYS_CPUSPEED		0x161		/* 400MHz;L=1 M=3 T=1 */

/*
 * Stack sizes
 */
#define	CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef	CONFIG_USE_IRQ
#define	CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
#define	CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

/*
 * DRAM Map
 */
#define	CONFIG_NR_DRAM_BANKS		1		/* 1 bank of DRAM */
#define	PHYS_SDRAM_1			0xa0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x04000000	/* 64 MB */

#define	CONFIG_SYS_DRAM_BASE		0xa0000000	/* CS0 */
#define	CONFIG_SYS_DRAM_SIZE		0x04000000	/* 64 MB DRAM */

#define	CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define	CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_DRAM_BASE

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		0xfffff800

/*
 * NOR FLASH
 */
#ifdef	CONFIG_CMD_FLASH
#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */
#define	PHYS_FLASH_SIZE			0x01000000	/* 16 MB */
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_FLASH_CFI_DRIVER		1
#define	CONFIG_SYS_FLASH_CFI_WIDTH      FLASH_CFI_32BIT

#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_MAX_FLASH_SECT	64

#define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_LOCK_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_UNLOCK_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_PROTECTION

#define	CONFIG_ENV_IS_IN_FLASH		1
#define	CONFIG_ENV_SECT_SIZE		0x40000
#else
#define	CONFIG_SYS_NO_FLASH
#define	CONFIG_ENV_IS_NOWHERE
#endif

#define	CONFIG_SYS_MONITOR_BASE		0x0
#define	CONFIG_SYS_MONITOR_LEN		0x40000

#define	CONFIG_ENV_SIZE			0x4000
#define	CONFIG_ENV_ADDR			0x40000

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GAFR0_L_VAL	0x00011004
#define	CONFIG_SYS_GAFR0_U_VAL	0xa5000008
#define	CONFIG_SYS_GAFR1_L_VAL	0x60888050
#define	CONFIG_SYS_GAFR1_U_VAL	0xaaa50aaa
#define	CONFIG_SYS_GAFR2_L_VAL	0x0aaaaaaa
#define	CONFIG_SYS_GAFR2_U_VAL	0x00000000
#define	CONFIG_SYS_GPCR0_VAL	0x0
#define	CONFIG_SYS_GPCR1_VAL	0x0
#define	CONFIG_SYS_GPCR2_VAL	0x0
#define	CONFIG_SYS_GPDR0_VAL	0xcfff8140
#define	CONFIG_SYS_GPDR1_VAL	0xfcbfbef3
#define	CONFIG_SYS_GPDR2_VAL	0x0001ffff
#define	CONFIG_SYS_GPSR0_VAL	0x00010f8f
#define	CONFIG_SYS_GPSR1_VAL	0x00bf5de5
#define	CONFIG_SYS_GPSR2_VAL	0x03fe0800

#define	CONFIG_SYS_PSSR_VAL	PSSR_RDH

/* Clock setup:
 * CKEN[1] - PWM1 ; CKEN[6] - FFUART
 * CKEN[12] - MMC ; CKEN[16] - LCD
 */
#define	CONFIG_SYS_CKEN		0x00011042
#define	CONFIG_SYS_CCCR		0x00000161

/*
 * Memory settings
 */
#define	CONFIG_SYS_MSC0_VAL	0x800092c2
#define	CONFIG_SYS_MSC1_VAL	0x80008000
#define	CONFIG_SYS_MSC2_VAL	0x80008000
#define	CONFIG_SYS_MDCNFG_VAL	0x00001ac9
#define	CONFIG_SYS_MDREFR_VAL	0x00118018
#define	CONFIG_SYS_MDMRS_VAL	0x00220032
#define	CONFIG_SYS_FLYCNFG_VAL	0x01fe01fe
#define	CONFIG_SYS_SXCNFG_VAL	0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000000
#define	CONFIG_SYS_MCMEM0_VAL	0x00010504
#define	CONFIG_SYS_MCMEM1_VAL	0x00010504
#define	CONFIG_SYS_MCATT0_VAL	0x00010504
#define	CONFIG_SYS_MCATT1_VAL	0x00010504
#define	CONFIG_SYS_MCIO0_VAL	0x00010e04
#define	CONFIG_SYS_MCIO1_VAL	0x00010e04

#endif	/* __CONFIG_H */
