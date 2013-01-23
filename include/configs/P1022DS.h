/*
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/freescale/common/ics307_clk.h"

#ifdef CONFIG_36BIT
#define CONFIG_PHYS_64BIT
#endif

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD
#define CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH
#define CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE			/* BOOKE */
#define CONFIG_E500			/* BOOKE e500 family */
#define CONFIG_MPC85xx			/* MPC8540/60/55/41/48 */
#define CONFIG_P1022
#define CONFIG_P1022DS
#define CONFIG_MP			/* support multiple processors */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xeff80000
#endif

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#define CONFIG_FSL_ELBC			/* Has Enhanced localbus controller */
#define CONFIG_PCI			/* Enable PCI/PCIE */
#define CONFIG_PCIE1			/* PCIE controler 1 (slot 1) */
#define CONFIG_PCIE2			/* PCIE controler 2 (slot 2) */
#define CONFIG_PCIE3			/* PCIE controler 3 (ULI bridge) */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET		/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_FSL_LAW			/* Use common FSL init code */

#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk()
#define CONFIG_DDR_CLK_FREQ	get_board_ddr_clk()
#define CONFIG_ICS307_REFCLK_HZ	33333000  /* ICS307 clock chip ref freq */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE
#define CONFIG_BTB

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		0x7fffffff

#define CONFIG_SYS_CCSRBAR		0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_DDR_SPD
#define CONFIG_VERY_BIG_RAM
#define CONFIG_FSL_DDR3

#ifdef CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#endif

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/* I2C addresses of SPD EEPROMs */
#define CONFIG_SYS_SPD_BUS_NUM		1
#define SPD_EEPROM_ADDRESS		0x51	/* CTLR 0 DIMM 0 */

/*
 * Memory map
 *
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 * 0x8000_0000	0xdfff_ffff	PCI Express Mem		1.5G non-cacheable
 * 0xffc0_0000	0xffc2_ffff	PCI IO range		192K non-cacheable
 *
 * Localbus cacheable (TBD)
 * 0xXXXX_XXXX	0xXXXX_XXXX	SRAM			YZ M Cacheable
 *
 * Localbus non-cacheable
 * 0xe000_0000	0xe80f_ffff	Promjet/free		128M non-cacheable
 * 0xe800_0000	0xefff_ffff	FLASH			128M non-cacheable
 * 0xffdf_0000	0xffdf_7fff	PIXIS			32K non-cacheable TLB0
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_FLASH_BASE		0xe0000000 /* start of FLASH 128M */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_FLASH_BR_PRELIM  \
	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000) | BR_PS_16 | BR_V)
#define CONFIG_FLASH_OR_PRELIM	(OR_AM_128MB | 0xff7)

#define CONFIG_SYS_BR0_PRELIM	CONFIG_FLASH_BR_PRELIM  /* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM	CONFIG_FLASH_OR_PRELIM  /* NOR Options */

#define CONFIG_SYS_BR1_PRELIM	\
	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | BR_PS_16 | BR_V)
#define CONFIG_SYS_OR1_PRELIM	CONFIG_FLASH_OR_PRELIM

#define CONFIG_SYS_FLASH_BANKS_LIST	\
	{CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000, CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	2
#define CONFIG_SYS_MAX_FLASH_SECT	1024

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* start of monitor */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R
#define CONFIG_HWCONFIG

#define CONFIG_FSL_NGPIXIS
#define PIXIS_BASE		0xffdf0000	/* PIXIS registers */
#ifdef CONFIG_PHYS_64BIT
#define PIXIS_BASE_PHYS		0xfffdf0000ull
#else
#define PIXIS_BASE_PHYS		PIXIS_BASE
#endif

#define CONFIG_SYS_BR2_PRELIM	(BR_PHYS_ADDR(PIXIS_BASE_PHYS) | BR_PS_8 | BR_V)
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_32KB | 0x6ff7)

#define PIXIS_LBMAP_SWITCH	7
#define PIXIS_LBMAP_MASK	0xF0
#define PIXIS_LBMAP_ALTBANK	0x20
#define PIXIS_ELBC_SPI_MASK	0xc0
#define PIXIS_SPI		0x80

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000 /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000 /* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* Video */
#define CONFIG_FSL_DIU_FB

#ifdef CONFIG_FSL_DIU_FB
#define CONFIG_SYS_DIU_ADDR	(CONFIG_SYS_CCSRBAR + 0x10000)
#define CONFIG_VIDEO
#define CONFIG_CMD_BMP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
/*
 * With CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS, flash I/O is really slow, so
 * disable empty flash sector detection, which is I/O-intensive.
 */
#undef CONFIG_SYS_FLASH_EMPTY_INFO
#endif

#ifndef CONFIG_FSL_DIU_FB
#define CONFIG_ATI
#endif

#ifdef CONFIG_ATI
#define VIDEO_IO_OFFSET		CONFIG_SYS_PCIE1_IO_VIRT
#define CONFIG_VIDEO
#define CONFIG_BIOSEMU
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_ATI_RADEON_FB
#define CONFIG_VIDEO_LOGO
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS VIDEO_IO_OFFSET
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#endif

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

/* new uImage format support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE

/* I2C */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_NOPROBES		{{0, 0x29}}
#define CONFIG_SYS_I2C_OFFSET		0x3000
#define CONFIG_SYS_I2C2_OFFSET		0x3100

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_BUS_NUM	1

/*
 * eSPI - Enhanced SPI
 */
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION

#define CONFIG_HARD_SPI
#define CONFIG_FSL_ESPI

#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		10000000
#define CONFIG_SF_DEFAULT_MODE		0

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, Slot 2, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc20000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xfffc20000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc20000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, direct to uli, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xfffc10000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc10000
#endif
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 3, Slot 1, tgtid 3, Base address b000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE3_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_IO_PHYS	0xfffc00000ull
#else
#define CONFIG_SYS_PCIE3_IO_PHYS	0xffc00000
#endif
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

#ifdef CONFIG_PCI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_E1000			/* Define e1000 pci Ethernet card */
#endif

/* SATA */
#define CONFIG_LIBATA
#define CONFIG_FSL_SATA
#define CONFIG_FSL_SATA_V2

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#ifdef CONFIG_FSL_SATA
#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#endif

#define CONFIG_MMC
#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#endif

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI)
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

#define CONFIG_TSEC_ENET
#ifdef CONFIG_TSEC_ENET

#define CONFIG_TSECV2

#define CONFIG_MII			/* MII PHY management */
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"

#define TSEC1_PHY_ADDR		1
#define TSEC2_PHY_ADDR		2

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_PHY_GIGE		/* Include GbE speed/duplex detection */
#endif

/*
 * Environment
 */
#ifdef CONFIG_SYS_RAMBOOT
#ifdef CONFIG_RAMBOOT_SPIFLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS	0
#define CONFIG_ENV_SPI_CS	0
#define CONFIG_ENV_SPI_MAX_HZ	10000000
#define CONFIG_ENV_SPI_MODE	0
#define CONFIG_ENV_SIZE		0x2000	/* 8KB */
#define CONFIG_ENV_OFFSET	0x100000	/* 1MB */
#define CONFIG_ENV_SECT_SIZE	0x10000
#elif defined(CONFIG_RAMBOOT_SDCARD)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_SYS_MMC_ENV_DEV	0
#elif defined(CONFIG_NAND_U_BOOT)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE		CONFIG_SYS_NAND_BLOCK_SIZE
#define CONFIG_ENV_OFFSET	((512 * 1024) + CONFIG_SYS_NAND_BLOCK_SIZE)
#define CONFIG_ENV_RANGE	(3 * CONFIG_ENV_SIZE)
#else
#define CONFIG_ENV_IS_NOWHERE	/* Store ENV in memory only */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif
#else
#define CONFIG_ENV_IS_IN_FLASH
#if CONFIG_SYS_MONITOR_BASE > 0xfff80000
#define CONFIG_ENV_ADDR	0xfff80000
#else
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#endif
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_ERRATA
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_CMD_NET
#endif

/*
 * USB
 */
#define CONFIG_HAS_FSL_DR_USB
#ifdef CONFIG_HAS_FSL_DR_USB
#define CONFIG_USB_EHCI

#ifdef CONFIG_USB_EHCI
#define CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_FAT
#endif
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING			/* Command-line editing */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_HZ		1000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#define CONFIG_HOSTNAME		p1022ds
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY	10	/* -1 disables auto-boot */

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"		\
	"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"	\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
		"protect off $ubootaddr +$filesize && "		\
		"erase $ubootaddr +$filesize && "		\
		"cp.b $loadaddr $ubootaddr $filesize && "	\
		"protect on $ubootaddr +$filesize && "		\
		"cmp.b $loadaddr $ubootaddr $filesize\0"	\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"			\
	"fdtaddr=c00000\0"	  			      	\
	"fdtfile=p1022ds.dtb\0"	  				\
	"bdev=sda3\0"		  			      	\
	"hwconfig=esdhc;audclk:12\0"

#define CONFIG_HDBOOT					\
	"setenv bootargs root=/dev/$bdev rw "		\
	"console=$consoledev,$baudrate $othbootargs $videobootargs;"	\
	"tftp $loadaddr $bootfile;"			\
	"tftp $fdtaddr $fdtfile;"			\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs $videobootargs;"	\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs $videobootargs;"	\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_RAMBOOTCOMMAND

#endif
