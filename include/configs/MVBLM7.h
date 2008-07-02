/*
 * Copyright (C) Matrix Vision GmbH 2008
 *
 * Matrix Vision mvBlueLYNX-M7 configuration file
 * based on Freescale's MPC8349ITX.
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

#include <version.h>

/*
 * High Level Configuration Options
 */
#define CONFIG_E300	1
#define CONFIG_MPC83XX	1
#define CONFIG_MPC834X	1
#define CONFIG_MPC8343	1

#define CFG_IMMR		0xE0000000

#define CONFIG_PCI
#define CONFIG_83XX_GENERIC_PCI
#define CONFIG_PCI_SKIP_HOST_BRIDGE
#define CONFIG_HARD_I2C
#define CONFIG_TSEC_ENET
#define CONFIG_MPC8XXX_SPI
#define CONFIG_HARD_SPI
#define MVBLM7_MMC_CS   0x04000000

/* I2C */
#undef CONFIG_SOFT_I2C

#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100

#define CFG_I2C_SPEED		100000
#define CFG_I2C_SLAVE		0x7F

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE
#define CFG_83XX_DDR_USES_CS0	1
#define CFG_MEMTEST_START	(60<<20)
#define CFG_MEMTEST_END		(70<<20)

#define CFG_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05

#define CFG_DDR_SIZE		256

/* HC, 75Ohm, DDR-II, DRQ */
#define CFG_DDRCDR		0x80000001
/* EN, ODT_WR, 3BA, 14row, 10col */
#define CFG_DDR_CS0_CONFIG	0x80014102
#define CFG_DDR_CS1_CONFIG	0x0
#define CFG_DDR_CS2_CONFIG	0x0
#define CFG_DDR_CS3_CONFIG	0x0

#define CFG_DDR_CS0_BNDS	0x0000000f
#define CFG_DDR_CS1_BNDS	0x0
#define CFG_DDR_CS2_BNDS	0x0
#define CFG_DDR_CS3_BNDS	0x0

#define CFG_DDR_CLK_CNTL	0x02000000

#define CFG_DDR_TIMING_0	0x00260802
#define CFG_DDR_TIMING_1	0x2625b221
#define CFG_DDR_TIMING_2	0x1f9820c7
#define CFG_DDR_TIMING_3	0x00000000

/* ~MEM_EN, SREN, DDR-II, 32_BE */
#define CFG_DDR_SDRAM_CFG	0x43080000
#define CFG_DDR_SDRAM_CFG2	0x00401000
#define CFG_DDR_INTERVAL	0x04060100

#define CFG_DDR_MODE		0x078e0232

/* Flash */
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CFG_FLASH_BASE		0xFF800000
#define CFG_FLASH_SIZE		8
#define CFG_FLASH_SIZE_SHIFT	3
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_ERASE_TOUT	60000
#define CFG_FLASH_WRITE_TOUT	500
#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	256

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BR_PS_16 | BR_V)
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM |  \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_XACS|\
				OR_GPCM_SCY_15 | OR_GPCM_TRLX | OR_GPCM_EHTR | \
				OR_GPCM_EAD)
#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE
#define CFG_LBLAWAR0_PRELIM	(LBLAWAR_EN | (0x13 + CFG_FLASH_SIZE_SHIFT))

/*
 * U-Boot memory configuration
 */
#define CFG_MONITOR_BASE	TEXT_BASE
#undef	CFG_RAMBOOT

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0xFD000000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000		/* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE	0x100		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(512 * 1024)
#define CFG_MALLOC_LEN		(512 * 1024)

/*
 * Local Bus LCRR and LBCR regs
 *  LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *  CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CFG_LCRR	(LCRR_DBYP | LCRR_CLKDIV_4)
#define CFG_LBC_LBCR	0x00000000

/* LB sdram refresh timer, about 6us */
#define CFG_LBC_LSRT	0x32000000
/* LB refresh timer prescal, 266MHz/32*/
#define CFG_LBC_MRTPR	0x20000000

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_CONSOLE		ttyS0
#define CONFIG_BAUDRATE		115200

#define CFG_NS16550_COM1	(CFG_IMMR + 0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR + 0x4600)

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1
#define MV_DTB_NAME	"mvblm7.dtb"

/*
 * PCI
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x10000000
#define CFG_PCI1_MMIO_BASE	(CFG_PCI1_MEM_BASE + CFG_PCI1_MEM_SIZE)
#define CFG_PCI1_MMIO_PHYS	CFG_PCI1_MMIO_BASE
#define CFG_PCI1_MMIO_SIZE	0x10000000
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xE2000000
#define CFG_PCI1_IO_SIZE	0x01000000

#define _IO_BASE		0x00000000

#define CONFIG_NET_MULTI	1
#define CONFIG_NET_RETRY_COUNT	3

#define PCI_66M
#define CONFIG_83XX_CLKIN	66666667
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW

/* TSEC */
#define CONFIG_GMII
#define CFG_VSC8601_SKEWFIX
#define	CFG_VSC8601_SKEW_TX	3
#define	CFG_VSC8601_SKEW_RX	3

#define CONFIG_TSEC1
#define CONFIG_TSEC2

#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_FEC1_PHY_NORXERR
#define CFG_TSEC1_OFFSET	0x24000
#define CFG_TSEC1		(CFG_IMMR+CFG_TSEC1_OFFSET)
#define TSEC1_PHY_ADDR		0x10
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT|TSEC_REDUCED)

#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME  	"TSEC1"
#define CONFIG_FEC2_PHY_NORXERR
#define CFG_TSEC2_OFFSET	0x25000
#define CFG_TSEC2 		(CFG_IMMR+CFG_TSEC2_OFFSET)
#define TSEC2_PHY_ADDR		0x11
#define TSEC2_PHYIDX		0
#define TSEC2_FLAGS		(TSEC_GIGABIT|TSEC_REDUCED)

#define CONFIG_ETHPRIME		"TSEC0"

#define CONFIG_BOOTP_VENDOREX
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_RANDOM_DELAY
#define CONFIG_BOOTP_SEND_HOSTNAME

/* USB */
#define CONFIG_HAS_FSL_DR_USB

/*
 * Environment
 */
#undef  CFG_FLASH_PROTECTION
#define CONFIG_ENV_OVERWRITE

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0xFF800000
#define CFG_ENV_SIZE		0x2000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND 	CFG_ENV_SIZE

#define CONFIG_LOADS_ECHO
#define CFG_LOADS_BAUD_CHANGE

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_PCI
#define CONFIG_CMD_I2C
#define CONFIG_CMD_FPGA

#undef CONFIG_WATCHDOG

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "

/* default load address */
#define CFG_LOAD_ADDR	0x2000000
/* default location for tftp and bootm */
#define CONFIG_LOADADDR	0x200000

#define CFG_PROMPT	"mvBL-M7> "
#define CFG_CBSIZE	256

#define CFG_PBSIZE	(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS	16
#define CFG_BARGSIZE	CFG_CBSIZE
#define CFG_HZ		1000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

#define CFG_HRCW_LOW	0x0
#define CFG_HRCW_HIGH	0x0

/*
 * System performance
 */
#define CFG_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */
#define CFG_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CFG_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */

/* clocking */
#define CFG_SCCR_ENCCM		0
#define CFG_SCCR_USBMPHCM	0
#define	CFG_SCCR_USBDRCM	2
#define CFG_SCCR_TSEC1CM	1
#define CFG_SCCR_TSEC2CM	1

#define CFG_SICRH	0x1fff8003
#define CFG_SICRL	(SICRL_LDP_A | SICRL_USB1 | SICRL_USB0)

#define CFG_HID0_INIT	0x000000000
#define CFG_HID0_FINAL	CFG_HID0_INIT

#define CFG_HID2	HID2_HBE
#define CONFIG_HIGH_BATS	1

/* DDR  */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI  */
#define CFG_IBAT1L	(CFG_PCI1_MEM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_PCI1_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_PCI1_MMIO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT |\
				BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_PCI1_MMIO_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* no PCI2 */
#define CFG_IBAT3L	0
#define CFG_IBAT3U	0
#define CFG_IBAT4L	0
#define CFG_IBAT4U	0

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CFG_IBAT5L	(CFG_IMMR | BATL_PP_10 | BATL_CACHEINHIBIT | \
				BATL_GUARDEDSTORAGE)
#define CFG_IBAT5U	(CFG_IMMR | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE 0xFDF00000 & FLASH @ 0xFF800000 */
#define CFG_IBAT6L	(0xF0000000 | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT7L	0
#define CFG_IBAT7U	0

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U
#define CFG_DBAT4L	CFG_IBAT4L
#define CFG_DBAT4U	CFG_IBAT4U
#define CFG_DBAT5L	CFG_IBAT5L
#define CFG_DBAT5U	CFG_IBAT5U
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot */


/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_NETDEV		eth0

/* Default path and filenames */
#define CONFIG_BOOTDELAY		5
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR	"s"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY		1000

#define MV_CI			"mvBL-M7"
#define MV_VCI			"mvBL-M7"
#define MV_FPGA_DATA		"0xfff80000"
#define MV_FPGA_SIZE		"0x76ca2"
#define MV_KERNEL_ADDR		"0xff810000"
#define MV_INITRD_ADDR		"0xffc00000"
#define MV_AUTOSCR_ADDR		"0xff804000"
#define MV_AUTOSCR_ADDR2	"0xff806000"
#define MV_DTB_ADDR		"0xff808000"
#define MV_INITRD_LENGTH	"0x00300000"

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define MV_KERNEL_ADDR_RAM	"0x00100000"
#define MV_DTB_ADDR_RAM		"0x00600000"
#define MV_INITRD_ADDR_RAM	"0x01000000"

#define CONFIG_BOOTCOMMAND	"if imi ${autoscr_addr}; \
					then autoscr ${autoscr_addr};  \
					else autoscr ${autoscr_addr2}; \
				fi;"
#define CONFIG_BOOTARGS		"root=/dev/ram ro rootfstype=squashfs"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"console_nr=0\0"					\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"fpga=0\0"						\
	"fpgadata=" MV_FPGA_DATA "\0"				\
	"fpgadatasize=" MV_FPGA_SIZE "\0"			\
	"autoscr_addr=" MV_AUTOSCR_ADDR "\0"			\
	"autoscr_addr2=" MV_AUTOSCR_ADDR2 "\0"			\
	"mv_kernel_addr=" MV_KERNEL_ADDR "\0"			\
	"mv_kernel_addr_ram=" MV_KERNEL_ADDR_RAM "\0"		\
	"mv_initrd_addr=" MV_INITRD_ADDR "\0"			\
	"mv_initrd_addr_ram=" MV_INITRD_ADDR_RAM "\0"		\
	"mv_initrd_length=" MV_INITRD_LENGTH "\0"		\
	"mv_dtb_addr=" MV_DTB_ADDR "\0"				\
	"mv_dtb_addr_ram=" MV_DTB_ADDR_RAM "\0"			\
	"dtb_name=" MV_DTB_NAME "\0"				\
	"mv_version=" U_BOOT_VERSION "\0"			\
	"dhcp_client_id=" MV_CI "\0"				\
	"dhcp_vendor-class-identifier=" MV_VCI "\0"		\
	"netretry=no\0"						\
	"use_static_ipaddr=no\0"				\
	"static_ipaddr=192.168.90.10\0"				\
	"static_netmask=255.255.255.0\0"			\
	"static_gateway=0.0.0.0\0"				\
	"initrd_name=uInitrd.mvblm7-xenorfs\0"			\
	"zcip=no\0"						\
	"netboot=yes\0"						\
	"mvtest=Ff\0"						\
	"tried_bootfromflash=no\0"				\
	"tried_bootfromnet=no\0"				\
	"bootfile=mvblm72625.boot\0"				\
	"use_dhcp=yes\0"					\
	"gev_start=yes\0"					\
	"mvbcdma_debug=0\0"					\
	"mvbcia_debug=0\0"					\
	"propdev_debug=0\0"					\
	"gevss_debug=0\0"					\
	"watchdog=0\0"						\
	"usb_dr_mode=host\0"					\
	""

#define CONFIG_FPGA_COUNT	1
#define CONFIG_FPGA		CFG_ALTERA_CYCLON2
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_CYCLON2

#endif
