/*
 * Voipac PXA270 configuration file
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
#define	CONFIG_VPAC270		1	/* Voipac PXA270 board */
#define	CONFIG_SYS_TEXT_BASE	0xa0000000

#ifdef	CONFIG_ONENAND
#define	CONFIG_SPL
#define	CONFIG_SPL_ONENAND_SUPPORT
#define	CONFIG_SPL_ONENAND_LOAD_ADDR	0x2000
#define	CONFIG_SPL_ONENAND_LOAD_SIZE	\
	(512 * 1024 - CONFIG_SPL_ONENAND_LOAD_ADDR)
#define	CONFIG_SPL_TEXT_BASE	0x5c000000
#define	CONFIG_SPL_LDSCRIPT	"board/vpac270/u-boot-spl.lds"
#endif

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128*1024)
#define	CONFIG_ARCH_CPU_INIT
#define	CONFIG_BOOTCOMMAND						\
	"if mmc init && fatload mmc 0 0xa4000000 uImage; then "		\
		"bootm 0xa4000000; "					\
	"fi; "								\
	"if usb reset && fatload usb 0 0xa4000000 uImage; then "	\
		"bootm 0xa4000000; "					\
	"fi; "								\
	"if ide reset && fatload ide 0 0xa4000000 uImage; then "	\
		"bootm 0xa4000000; "					\
	"fi; "								\
	"bootm 0x60000;"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"update_onenand="						\
		"onenand erase 0x0 0x80000 ; "				\
		"onenand write 0xa0000000 0x0 0x80000"

#define	CONFIG_BOOTARGS			"console=tty0 console=ttyS0,115200"
#define	CONFIG_TIMESTAMP
#define	CONFIG_BOOTDELAY		2	/* Autoboot delay */
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS
#define	CONFIG_LZMA			/* LZMA compression support */
#define	CONFIG_OF_LIBFDT

/*
 * Serial Console Configuration
 */
#define	CONFIG_PXA_SERIAL
#define	CONFIG_FFUART			1
#define	CONFIG_BAUDRATE			115200

/*
 * Bootloader Components Configuration
 */
#include <config_cmd_default.h>

#define	CONFIG_CMD_NET
#define	CONFIG_CMD_ENV
#undef	CONFIG_CMD_IMLS
#define	CONFIG_CMD_MMC
#define	CONFIG_CMD_USB
#undef	CONFIG_LCD
#define	CONFIG_CMD_IDE

#ifdef	CONFIG_ONENAND
#undef	CONFIG_CMD_FLASH
#define	CONFIG_CMD_ONENAND
#else
#define	CONFIG_CMD_FLASH
#undef	CONFIG_CMD_ONENAND
#endif

/*
 * Networking Configuration
 *  chip on the Voipac PXA270 board
 */
#ifdef	CONFIG_CMD_NET
#define	CONFIG_CMD_PING
#define	CONFIG_CMD_DHCP

#define	CONFIG_DRIVER_DM9000		1
#define	CONFIG_DM9000_BASE		0x08000300	/* CS2 */
#define	DM9000_IO			(CONFIG_DM9000_BASE)
#define	DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define	CONFIG_NET_RETRY_COUNT		10

#define	CONFIG_BOOTP_BOOTFILESIZE
#define	CONFIG_BOOTP_BOOTPATH
#define	CONFIG_BOOTP_GATEWAY
#define	CONFIG_BOOTP_HOSTNAME
#endif

/*
 * MMC Card Configuration
 */
#ifdef	CONFIG_CMD_MMC
#define	CONFIG_MMC
#define	CONFIG_GENERIC_MMC
#define	CONFIG_PXA_MMC_GENERIC
#define	CONFIG_SYS_MMC_BASE		0xF0000000
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_EXT2
#define	CONFIG_DOS_PARTITION
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
#define	CONFIG_CMDLINE_EDITING		1
#define	CONFIG_AUTO_COMPLETE		1

/*
 * Clock Configuration
 */
#define	CONFIG_SYS_HZ			1000		/* Timer @ 3250000 Hz */
#define	CONFIG_SYS_CPUSPEED		0x190		/* 312MHz */


/*
 * DRAM Map
 */
#define	CONFIG_NR_DRAM_BANKS		2		/* 2 banks of DRAM */
#define	PHYS_SDRAM_1			0xa0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x08000000	/* 128 MB */

#ifdef	CONFIG_RAM_256M
#define	PHYS_SDRAM_2			0x80000000	/* SDRAM Bank #2 */
#define	PHYS_SDRAM_2_SIZE		0x08000000	/* 128 MB */
#endif

#define	CONFIG_SYS_DRAM_BASE		0xa0000000	/* CS0 */
#ifdef	CONFIG_RAM_256M
#define	CONFIG_SYS_DRAM_SIZE		0x10000000	/* 256 MB DRAM */
#else
#define	CONFIG_SYS_DRAM_SIZE		0x08000000	/* 128 MB DRAM */
#endif

#define	CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define	CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		0x5c010000

/*
 * NOR FLASH
 */
#define	CONFIG_SYS_MONITOR_BASE		0x0
#define	CONFIG_SYS_MONITOR_LEN		0x80000
#define	CONFIG_ENV_ADDR			\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_ENV_SIZE			0x20000
#define	CONFIG_ENV_SECT_SIZE		0x20000

#if	defined(CONFIG_CMD_FLASH)	/* NOR */
#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */

#ifdef	CONFIG_RAM_256M
#define	PHYS_FLASH_2			0x02000000	/* Flash Bank #2 */
#endif

#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_FLASH_CFI_DRIVER		1

#define	CONFIG_SYS_MAX_FLASH_SECT	(4 + 255)
#ifdef	CONFIG_RAM_256M
#define	CONFIG_SYS_MAX_FLASH_BANKS	2
#define	CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1, PHYS_FLASH_2 }
#else
#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#endif

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ)

#define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#define	CONFIG_SYS_FLASH_PROTECTION		1

#define	CONFIG_ENV_IS_IN_FLASH		1

#elif	defined(CONFIG_CMD_ONENAND)	/* OneNAND */
#define	CONFIG_SYS_NO_FLASH
#define	CONFIG_SYS_ONENAND_BASE		0x00000000

#define	CONFIG_ENV_IS_IN_ONENAND	1

#else	/* No flash */
#define	CONFIG_SYS_NO_FLASH
#define	CONFIG_SYS_ENV_IS_NOWHERE
#endif

/*
 * IDE
 */
#ifdef	CONFIG_CMD_IDE
#define	CONFIG_LBA48
#undef	CONFIG_IDE_LED
#undef	CONFIG_IDE_RESET

#define	__io

#define	CONFIG_SYS_IDE_MAXBUS		1
#define	CONFIG_SYS_IDE_MAXDEVICE	1

#define	CONFIG_SYS_ATA_BASE_ADDR	0x0c000000
#define	CONFIG_SYS_ATA_IDE0_OFFSET	0x0

#define	CONFIG_SYS_ATA_DATA_OFFSET	0x120
#define	CONFIG_SYS_ATA_REG_OFFSET	0x120
#define	CONFIG_SYS_ATA_ALT_OFFSET	0x120

#define	CONFIG_SYS_ATA_STRIDE		2
#endif

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GPSR0_VAL	0x01308800
#define	CONFIG_SYS_GPSR1_VAL	0x00cf0000
#define	CONFIG_SYS_GPSR2_VAL	0x922ac000
#define	CONFIG_SYS_GPSR3_VAL	0x0161e800

#define	CONFIG_SYS_GPCR0_VAL	0x00010000
#define	CONFIG_SYS_GPCR1_VAL	0x0
#define	CONFIG_SYS_GPCR2_VAL	0x0
#define	CONFIG_SYS_GPCR3_VAL	0x0

#define	CONFIG_SYS_GPDR0_VAL	0xcbb18800
#define	CONFIG_SYS_GPDR1_VAL	0xfccfa981
#define	CONFIG_SYS_GPDR2_VAL	0x922affff
#define	CONFIG_SYS_GPDR3_VAL	0x0161e904

#define	CONFIG_SYS_GAFR0_L_VAL	0x00100000
#define	CONFIG_SYS_GAFR0_U_VAL	0xa5da8510
#define	CONFIG_SYS_GAFR1_L_VAL	0x6992901a
#define	CONFIG_SYS_GAFR1_U_VAL	0xaaa5a0aa
#define	CONFIG_SYS_GAFR2_L_VAL	0xaaaaaaaa
#define	CONFIG_SYS_GAFR2_U_VAL	0x4109a401
#define	CONFIG_SYS_GAFR3_L_VAL	0x54010310
#define	CONFIG_SYS_GAFR3_U_VAL	0x00025401

#define	CONFIG_SYS_PSSR_VAL	0x30

/*
 * Clock settings
 */
#define	CONFIG_SYS_CKEN		0x00500240
#define	CONFIG_SYS_CCCR		0x02000290

/*
 * Memory settings
 */
#define	CONFIG_SYS_MSC0_VAL	0x3ffc95fa
#define	CONFIG_SYS_MSC1_VAL	0x02ccf974
#define	CONFIG_SYS_MSC2_VAL	0x00000000
#ifdef	CONFIG_RAM_256M
#define	CONFIG_SYS_MDCNFG_VAL	0x8ad30ad3
#else
#define	CONFIG_SYS_MDCNFG_VAL	0x88000ad3
#endif
#define	CONFIG_SYS_MDREFR_VAL	0x201fe01e
#define	CONFIG_SYS_MDMRS_VAL	0x00000000
#define	CONFIG_SYS_FLYCNFG_VAL	0x00000000
#define	CONFIG_SYS_SXCNFG_VAL	0x40044004
#define	CONFIG_SYS_MEM_BUF_IMP	0x0f

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000001
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
#define	CONFIG_VOIPAC_LCD
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
#define	CONFIG_SYS_USB_OHCI_SLOT_NAME	"vpac270"
#define	CONFIG_USB_STORAGE
#endif

#endif	/* __CONFIG_H */
