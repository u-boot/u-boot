/*
 * Balloon3 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
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

/*
 * High Level Board Configuration Options
 */
#define	CONFIG_CPU_PXA27X		1	/* Marvell PXA270 CPU */
#define	CONFIG_BALLOON3		1	/* Balloon3 board */

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128*1024)
#define	CONFIG_ARCH_CPU_INIT
#define	CONFIG_BOOTCOMMAND						\
	"fpga load 0x0 0x50000 0x62638; "				\
	"if usb reset && fatload usb 0 0xa4000000 uImage; then "	\
		"bootm 0xa4000000; "					\
	"fi; "								\
	"bootm 0xd0000;"
#define	CONFIG_BOOTARGS			"console=tty0 console=ttyS2,115200"
#define	CONFIG_TIMESTAMP
#define	CONFIG_BOOTDELAY		2	/* Autoboot delay */
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS
#define	CONFIG_SYS_TEXT_BASE		0x0
#define	CONFIG_LZMA			/* LZMA compression support */

/*
 * Serial Console Configuration
 */
#define	CONFIG_PXA_SERIAL
#define	CONFIG_STUART			1
#define CONFIG_CONS_INDEX		2
#define	CONFIG_BAUDRATE			115200

/*
 * Bootloader Components Configuration
 */
#include <config_cmd_default.h>

#undef	CONFIG_CMD_NET
#undef	CONFIG_CMD_NFS
#undef	CONFIG_CMD_ENV
#undef	CONFIG_CMD_IMLS
#define	CONFIG_CMD_USB
#define	CONFIG_CMD_FPGA
#undef	CONFIG_LCD

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
#define	CONFIG_SYS_HZ			3250000		/* Timer @ 3250000 Hz */
#define	CONFIG_SYS_CPUSPEED		0x290		/* 520MHz */

/*
 * DRAM Map
 */
#define	CONFIG_NR_DRAM_BANKS		3		/* 2 banks of DRAM */
#define	PHYS_SDRAM_1			0xa0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x08000000	/* 128 MB */
#define	PHYS_SDRAM_2			0xb0000000	/* SDRAM Bank #2 */
#define	PHYS_SDRAM_2_SIZE		0x08000000	/* 128 MB */
#define	PHYS_SDRAM_3			0x80000000	/* SDRAM Bank #2 */
#define	PHYS_SDRAM_3_SIZE		0x08000000	/* 128 MB */

#define	CONFIG_SYS_DRAM_BASE		0xa0000000	/* CS0 */
#define	CONFIG_SYS_DRAM_SIZE		0x18000000	/* 384 MB DRAM */

#define	CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define	CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		0xa1000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		\
	(PHYS_SDRAM_1 + GENERATED_GBL_DATA_SIZE + 2048)

/*
 * NOR FLASH
 */
#ifdef	CONFIG_CMD_FLASH
#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */
#define	PHYS_FLASH_SIZE			0x00800000	/* 8 MB */
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_FLASH_CFI_DRIVER		1
#define	CONFIG_SYS_FLASH_CFI_WIDTH      FLASH_CFI_16BIT

#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_MAX_FLASH_SECT	256

#define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_LOCK_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_UNLOCK_TOUT	(2*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_PROTECTION
#define	CONFIG_ENV_IS_IN_FLASH
#else
#define	CONFIG_SYS_NO_FLASH
#define	CONFIG_SYS_ENV_IS_NOWHERE
#endif

#define	CONFIG_SYS_MONITOR_BASE		0x000000
#define	CONFIG_SYS_MONITOR_LEN		0x40000

#define	CONFIG_ENV_SIZE			0x2000
#define	CONFIG_ENV_ADDR			0x40000
#define	CONFIG_ENV_SECT_SIZE		0x10000

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GPSR0_VAL	0x307dc7fd
#define	CONFIG_SYS_GPSR1_VAL	0x03cffa4e
#define	CONFIG_SYS_GPSR2_VAL	0x7131c000
#define	CONFIG_SYS_GPSR3_VAL	0x01e1f3ff

#define	CONFIG_SYS_GPCR0_VAL	0x0
#define	CONFIG_SYS_GPCR1_VAL	0x0
#define	CONFIG_SYS_GPCR2_VAL	0x0
#define	CONFIG_SYS_GPCR3_VAL	0x0

#define	CONFIG_SYS_GPDR0_VAL	0xc0f98e02
#define	CONFIG_SYS_GPDR1_VAL	0xfcffa8b7
#define	CONFIG_SYS_GPDR2_VAL	0x22e3ffff
#define	CONFIG_SYS_GPDR3_VAL	0x000201fe

#define	CONFIG_SYS_GAFR0_L_VAL	0x96c00000
#define	CONFIG_SYS_GAFR0_U_VAL	0xa5e5459b
#define	CONFIG_SYS_GAFR1_L_VAL	0x699b759a
#define	CONFIG_SYS_GAFR1_U_VAL	0xaaa5a5aa
#define	CONFIG_SYS_GAFR2_L_VAL	0xaaaaaaaa
#define	CONFIG_SYS_GAFR2_U_VAL	0x01f9a6aa
#define	CONFIG_SYS_GAFR3_L_VAL	0x54510003
#define	CONFIG_SYS_GAFR3_U_VAL	0x00001599

#define	CONFIG_SYS_PSSR_VAL	0x30

/*
 * Clock settings
 */
#define	CONFIG_SYS_CKEN		0xffffffff
#define	CONFIG_SYS_CCCR		0x00000290

/*
 * Memory settings
 */
#define	CONFIG_SYS_MSC0_VAL	0x7ff07ff8
#define	CONFIG_SYS_MSC1_VAL	0x7ff07ff0
#define	CONFIG_SYS_MSC2_VAL	0x74a42491
#define	CONFIG_SYS_MDCNFG_VAL	0x89d309d3
#define	CONFIG_SYS_MDREFR_VAL	0x001d8018
#define	CONFIG_SYS_MDMRS_VAL	0x00220022
#define	CONFIG_SYS_FLYCNFG_VAL	0x00000000
#define	CONFIG_SYS_SXCNFG_VAL	0x00000000
#define	CONFIG_SYS_MEM_BUF_IMP	0x0f

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000000
#define	CONFIG_SYS_MCMEM0_VAL	0x00014307
#define	CONFIG_SYS_MCMEM1_VAL	0x00014307
#define	CONFIG_SYS_MCATT0_VAL	0x0001c787
#define	CONFIG_SYS_MCATT1_VAL	0x0001c787
#define	CONFIG_SYS_MCIO0_VAL	0x0001430f
#define	CONFIG_SYS_MCIO1_VAL	0x0001430f

/*
 * LCD
 */
#ifdef	CONFIG_LCD
#define	CONFIG_BALLOON3LCD
#define	CONFIG_VIDEO_LOGO
#define	CONFIG_CMD_BMP
#define	CONFIG_SPLASH_SCREEN
#define	CONFIG_SPLASH_SCREEN_ALIGN
#define	CONFIG_VIDEO_BMP_GZIP
#define	CONFIG_VIDEO_BMP_RLE8
#define	CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)
#endif

/*
 * USB
 */
#ifdef	CONFIG_CMD_USB
#define	CONFIG_USB_OHCI_NEW
#define	CONFIG_SYS_USB_OHCI_CPU_INIT
#define	CONFIG_SYS_USB_OHCI_BOARD_INIT
#define	CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define	CONFIG_SYS_USB_OHCI_REGS_BASE	0x4C000000
#define	CONFIG_SYS_USB_OHCI_SLOT_NAME	"balloon3"
#define	CONFIG_USB_STORAGE
#define	CONFIG_DOS_PARTITION
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_EXT2
#endif

/*
 * FPGA
 */
#ifdef	CONFIG_CMD_FPGA
#define	CONFIG_FPGA
#define	CONFIG_FPGA_XILINX
#define	CONFIG_FPGA_SPARTAN3
#define	CONFIG_SYS_FPGA_PROG_FEEDBACK
#define	CONFIG_SYS_FPGA_WAIT	1000
#define	CONFIG_MAX_FPGA_DEVICES	1
#endif

#endif	/* __CONFIG_H */
