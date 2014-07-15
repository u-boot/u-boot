/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011 Matrix Vision GmbH
 * Andre Schwarz <andre.schwarz@matrix-vision.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <version.h>

/*
 * High Level Configuration Options
 */
#define CONFIG_E300	1
#define CONFIG_MPC837x	1
#define CONFIG_MPC8377	1

#define CONFIG_SYS_TEXT_BASE	0xFC000000

#define CONFIG_PCI	1
#define CONFIG_PCI_INDIRECT_BRIDGE 1

#define	CONFIG_MASK_AER_AO
#define CONFIG_DISPLAY_AER_FULL

#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 */
#define CONFIG_TSEC_ENET

/*
 * System Clock Setup
 */
#define CONFIG_83XX_CLKIN	66666667 /* in Hz */
#define CONFIG_PCIE
#define CONFIG_83XX_GENERIC_PCIE_REGISTER_HOSES
#define CONFIG_SYS_CLK_FREQ	CONFIG_83XX_CLKIN

/*
 * Hardware Reset Configuration Word stored in EEPROM.
 */
#define CONFIG_SYS_HRCW_LOW	0
#define CONFIG_SYS_HRCW_HIGH	0

/* Arbiter Configuration Register */
#define CONFIG_SYS_ACR_PIPE_DEP	3
#define CONFIG_SYS_ACR_RPTCNT	3

/* System Priority Control Regsiter */
#define CONFIG_SYS_SPCR_TSECEP	3

/* System Clock Configuration Register */
#define CONFIG_SYS_SCCR_TSEC1CM		3
#define CONFIG_SYS_SCCR_TSEC2CM		0
#define CONFIG_SYS_SCCR_SDHCCM		3
#define CONFIG_SYS_SCCR_ENCCM		3 /* also clock for I2C-1 */
#define CONFIG_SYS_SCCR_USBDRCM		CONFIG_SYS_SCCR_ENCCM /* must match */
#define CONFIG_SYS_SCCR_PCIEXP1CM	3
#define CONFIG_SYS_SCCR_PCIEXP2CM	3
#define CONFIG_SYS_SCCR_PCICM		1
#define CONFIG_SYS_SCCR_SATACM		0xFF

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH	0x087c0000
#define CONFIG_SYS_SICRL	0x40000000

/*
 * Output Buffer Impedance
 */
#define CONFIG_SYS_OBIR		0x30000000

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_83XX_DDR_USES_CS0

#define CONFIG_SYS_DDRCDR_VALUE		(DDRCDR_EN | DDRCDR_PZ_HIZ |\
					 DDRCDR_NZ_HIZ | DDRCDR_ODT |\
					 DDRCDR_Q_DRN)

#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05

#define CONFIG_SYS_DDR_MODE_WEAK
#define CONFIG_SYS_DDR_WRITE_DATA_DELAY	2
#define CONFIG_SYS_DDR_CPO	0x1f

/* SPD table located at offset 0x20 in extended adressing ROM
 * used for HRCW fetch after power-on reset
 */
#define	CONFIG_SPD_EEPROM
#define	SPD_EEPROM_ADDRESS	0x50
#define	SPD_EEPROM_OFFSET	0x20
#define	SPD_EEPROM_ADDR_LEN	2

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(512*1024)
#define CONFIG_SYS_MALLOC_LEN		(512*1024)

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE -\
					 GENERATED_GBL_DATA_SIZE)

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_8
#define CONFIG_SYS_LBC_LBCR	0x00000000
#define CONFIG_FSL_ELBC		1

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_FLASH_SIZE		64

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_64MB)

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE | BR_PS_16 |\
				 BR_MS_GPCM | BR_V)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_FLASH_BASE | OR_UPM_XAM |\
				 OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 |\
				 OR_GPCM_XACS | OR_GPCM_SCY_15 |\
				 OR_GPCM_TRLX_SET | OR_GPCM_EHTR_SET |\
				 OR_GPCM_EAD)

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	512

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000 /* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500 /* Flash Write Timeout (ms) */

/*
 * NAND Flash on the Local Bus
 */
#define CONFIG_MTD_NAND_VERIFY_WRITE	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_FSL_ELBC		1

#define CONFIG_SYS_NAND_BASE	0xE0600000
#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_NAND_BASE | BR_DECC_CHK_GEN |\
				 BR_PS_8 | BR_MS_FCM | BR_V)
#define CONFIG_SYS_OR1_PRELIM	(OR_AM_32KB | OR_FCM_BCTLD | OR_FCM_CST |\
				 OR_FCM_CHT | OR_FCM_SCY_1 | OR_FCM_RST |\
				 OR_FCM_TRLX | OR_FCM_EHTR)

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

#define CONFIG_CONSOLE		ttyS0
#define CONFIG_BAUDRATE		115200

/* SERDES */
#define CONFIG_FSL_SERDES
#define CONFIG_FSL_SERDES1	0xe3000
#define CONFIG_FSL_SERDES2	0xe3100

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS 1

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI_MEM_BASE		0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS		CONFIG_SYS_PCI_MEM_BASE
#define CONFIG_SYS_PCI_MEM_SIZE		(256 << 20)
#define CONFIG_SYS_PCI_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI_MMIO_PHYS	CONFIG_SYS_PCI_MMIO_BASE
#define CONFIG_SYS_PCI_MMIO_SIZE	(256 << 20)
#define CONFIG_SYS_PCI_IO_BASE		0x00000000
#define CONFIG_SYS_PCI_IO_PHYS		0xE0300000
#define CONFIG_SYS_PCI_IO_SIZE		(1 << 20)

#ifdef CONFIG_PCIE
#define CONFIG_SYS_PCIE1_BASE		0xA0000000
#define CONFIG_SYS_PCIE1_CFG_BASE	0xA0000000
#define CONFIG_SYS_PCIE1_CFG_SIZE	(128 << 20)
#define CONFIG_SYS_PCIE1_MEM_BASE	0xA8000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xA8000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	(256 << 20)
#define CONFIG_SYS_PCIE1_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xB8000000
#define CONFIG_SYS_PCIE1_IO_SIZE	(8 << 20)

#define CONFIG_SYS_PCIE2_BASE		0xC0000000
#define CONFIG_SYS_PCIE2_CFG_BASE	0xC0000000
#define CONFIG_SYS_PCIE2_CFG_SIZE	(128 << 20)
#define CONFIG_SYS_PCIE2_MEM_BASE	0xC8000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xC8000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	(256 << 20)
#define CONFIG_SYS_PCIE2_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xD8000000
#define CONFIG_SYS_PCIE2_IO_SIZE	(8 << 20)
#endif

#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */

/*
 * TSEC
 */
#define CONFIG_GMII			/* MII PHY management */
#define CONFIG_SYS_VSC8601_SKEWFIX
#define	CONFIG_SYS_VSC8601_SKEW_TX	3
#define	CONFIG_SYS_VSC8601_SKEW_RX	3

#define CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET		0x24000
#define TSEC1_PHY_ADDR			0x10
#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC1_PHYIDX			0

#define CONFIG_ETHPRIME			"TSEC0"
#define CONFIG_HAS_ETH0

/*
 * SATA
 */
#define CONFIG_LIBATA
#define CONFIG_FSL_SATA

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1_OFFSET	0x18000
#define CONFIG_SYS_SATA1	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA1_OFFSET)
#define CONFIG_SYS_SATA1_FLAGS	FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2_OFFSET	0x19000
#define CONFIG_SYS_SATA2	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA2_OFFSET)
#define CONFIG_SYS_SATA2_FLAGS	FLAGS_DMA

#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_VENDOREX
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_RANDOM_DELAY
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_LIB_RAND

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_USB
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_SATA

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2

#define CONFIG_RBTREE
#define CONFIG_LZO

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT "nor0=NOR,nand0=NAND"
#define MTDPARTS_DEFAULT "mtdparts=NOR:1M(u-boot),2M(FPGA);NAND:-(root)"

#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE	1

#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LOAD_ADDR	0x2000000
#define CONFIG_LOADADDR		0x4000000
#define CONFIG_SYS_CBSIZE	256

#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#define CONFIG_LOADS_ECHO		1
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1

#define CONFIG_SYS_MEMTEST_START	(60<<20)
#define CONFIG_SYS_MEMTEST_END		(70<<20)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Core HID Setup
 */
#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK | \
				 HID0_ENABLE_INSTRUCTION_CACHE)
#define CONFIG_SYS_HID2		HID2_HBE

/*
 * MMU Setup
 */
#define CONFIG_HIGH_BATS	1

/* DDR: cache cacheable */
#define CONFIG_SYS_SDRAM	CONFIG_SYS_SDRAM_BASE

#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM | BATL_PP_RW |\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM | BATU_BL_256M | BATU_VS |\
				 BATU_VP)
#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

/* unused */
#define CONFIG_SYS_IBAT1L	(0)
#define CONFIG_SYS_IBAT1U	(0)
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* IMMRBAR, PCI IO and NAND: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_IMMR | BATL_PP_RW |\
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_IMMR | BATU_BL_8M | BATU_VS |\
				 BATU_VP)
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* unused */
#define CONFIG_SYS_IBAT3L	(0)
#define CONFIG_SYS_IBAT3U	(0)
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_FLASH_BASE | BATL_PP_RW |\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_FLASH_BASE | BATU_BL_64M |\
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L	(CONFIG_SYS_FLASH_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U

/* Stack in dcache: cacheable, no memory coherence */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K |\
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

/* PCI MEM space: cacheable */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_PCI_MEM_PHYS | BATL_PP_RW |\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_PCI_MEM_PHYS | BATU_BL_256M |\
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

/* PCI MMIO space: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT7L	(CONFIG_SYS_PCI_MMIO_PHYS | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT7U	(CONFIG_SYS_PCI_MMIO_PHYS | BATU_BL_256M |\
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

/*
 * I2C EEPROM settings
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_SIZE			0x4000

/*
 * Environment Configuration
 */
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0xFFD00000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/*
 * Video
 */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_SM501_PCI
#define VIDEO_FB_LITTLE_ENDIAN
#define CONFIG_CMD_BMP
#define CONFIG_VIDEO_SM501
#define CONFIG_VIDEO_SM501_32BPP
#define CONFIG_VIDEO_SM501_FBMEM_OFFSET	0x10000
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)

/*
 * SPI
 */
#define CONFIG_MPC8XXX_SPI

/*
 * USB
 */
#define CONFIG_SYS_USB_HOST
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_USB_STORAGE
#define CONFIG_USB_KEYBOARD
/*
 *
 */
#define CONFIG_BOOTDELAY	5
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR	"s"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY	1000

#define MV_CI		"MergerBox"
#define MV_VCI		"MergerBox"
#define MV_FPGA_DATA	0xfc100000
#define MV_FPGA_SIZE	0x00200000

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define MV_KERNEL_ADDR_RAM	0x02800000
#define MV_DTB_ADDR_RAM		0x00600000
#define MV_INITRD_ADDR_RAM	0x01000000
#define MV_FITADDR		0xfc300000
#define	MV_SPLAH_ADDR		0xffe00000

#define CONFIG_BOOTCOMMAND	"run i2c_init;if test ${boot_sqfs} -eq 1;"\
				"then; run fitboot;else;run ubiboot;fi;"
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console_nr=0\0"\
	"stdin=serial\0"\
	"stdout=serial\0"\
	"stderr=serial\0"\
	"boot_sqfs=1\0"\
	"usb_dr_mode=host\0"\
	"bootfile=MergerBox.fit\0"\
	"baudrate=" __stringify(CONFIG_BAUDRATE) "\0"\
	"fpga=0\0"\
	"fpgadata=" __stringify(MV_FPGA_DATA) "\0"\
	"fpgadatasize=" __stringify(MV_FPGA_SIZE) "\0"\
	"mv_kernel_ram=" __stringify(MV_KERNEL_ADDR_RAM) "\0"\
	"mv_initrd_ram=" __stringify(MV_INITRD_ADDR_RAM) "\0"\
	"mv_dtb_ram=" __stringify(MV_DTB_ADDR_RAM) "\0"\
	"uboota=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"\
	"fitaddr=" __stringify(MV_FITADDR) "\0"\
	"mv_version=" U_BOOT_VERSION "\0"\
	"mtdids=" MTDIDS_DEFAULT "\0"\
	"mtdparts=" MTDPARTS_DEFAULT "\0"\
	"dhcp_client_id=" MV_CI "\0"\
	"dhcp_vendor-class-identifier=" MV_VCI "\0"\
	"upd_uboot=dhcp;tftp bdi2000/u-boot-mergerbox-xp.bin;"\
		"protect off all;erase $uboota +0xC0000;"\
		"cp.b $loadaddr $uboota $filesize\0"\
	"upd_fpga=dhcp;tftp MergerBox.rbf;erase $fpgadata +$fpgadatasize;"\
		"cp.b $loadaddr $fpgadata $filesize\0"\
	"upd_fit=dhcp;tftp MergerBox.fit;erase $fitaddr +0x1000000;"\
		"cp.b $loadaddr $fitaddr $filesize\0"\
	"addsqshrfs=set bootargs $bootargs root=/dev/ram ro "\
		"rootfstype=squashfs\0"\
	"addubirfs=set bootargs $bootargs ubi.mtd=9 root=ubi0:rootfs rw "\
		"rootfstype=ubifs\0"\
	"addusbrfs=set bootargs $bootargs root=/dev/sda1 rw "\
		"rootfstype=ext3 usb-storage.delay_use=1 rootdelay=3\0"\
	"netusbboot=bootp;run fpganetload fitnetload addusbrfs doboot\0"\
	"netubiboot= bootp;run fpganetload fitnetload addubirfs doboot\0"\
	"ubiboot=run fitprep addubirfs;set mv_initrd_ram -;run doboot\0"\
	"doboot=bootm $mv_kernel_ram $mv_initrd_ram $mv_dtb_ram\0"\
	"fitprep=imxtract $fitaddr kernel $mv_kernel_ram;"\
		"imxtract $fitaddr ramdisk $mv_initrd_ram;"\
		"imxtract $fitaddr fdt $mv_dtb_ram\0"\
	"fdtprep=fdt addr $mv_dtb_ram;fdt boardsetup\0"\
	"fitboot=run fitprep fdtprep addsqshrfs doboot\0"\
	"i2c_init=run i2c_speed init_sdi_tx i2c_init_pll\0"\
	"i2c_init_pll=i2c mw 65 9 2;i2c mw 65 9 0;i2c mw 65 5 2b;"\
		"i2c mw 65 7 f;i2c mw 65 8 f;i2c mw 65 11 40;i2c mw 65 12 40;"\
		"i2c mw 65 13 40; i2c mw 65 14 40; i2c mw 65 a 0\0"\
	"i2c_speed=i2c dev 0;i2c speed 300000;i2c dev 1;i2c speed 120000\0"\
	"init_sdi_tx=i2c mw 21 6 0;i2c mw 21 2 0;i2c mw 21 3 0;sleep 1;"\
		"i2c mw 21 2 ff;i2c mw 21 3 3c\0"\
	"splashimage=" __stringify(MV_SPLAH_ADDR) "\0"\
	""

/*
 * FPGA
 */
#define CONFIG_FPGA_COUNT	1
#define CONFIG_FPGA
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_CYCLON2

#endif
