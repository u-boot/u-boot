/*
 * Copyright (C) Matrix Vision GmbH 2008
 *
 * Matrix Vision mvBlueLYNX-M7 configuration file
 * based on Freescale's MPC8349ITX.
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
#define CONFIG_MPC83xx	1
#define CONFIG_MPC834x	1
#define CONFIG_MPC8343	1

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#define CONFIG_SYS_IMMR		0xE0000000

#define CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCI_SKIP_HOST_BRIDGE
#define CONFIG_TSEC_ENET
#define CONFIG_MPC8XXX_SPI
#define CONFIG_HARD_SPI
#define MVBLM7_MMC_CS   0x04000000
#define CONFIG_MISC_INIT_R

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	100000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	100000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

/*
 * DDR Setup
 */
#undef	CONFIG_SPD_EEPROM

#define CONFIG_SYS_DDR_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_83XX_DDR_USES_CS0	1
#define CONFIG_SYS_MEMTEST_START	(60<<20)
#define CONFIG_SYS_MEMTEST_END		(70<<20)
#define CONFIG_VERY_BIG_RAM

#define CONFIG_SYS_DDRCDR		(DDRCDR_PZ_HIZ \
					| DDRCDR_NZ_HIZ \
					| DDRCDR_Q_DRN)
					/* 0x22000001 */
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05

#define CONFIG_SYS_DDR_SIZE		512

#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014202

#define CONFIG_SYS_DDR_CS0_BNDS		0x0000003f

#define CONFIG_SYS_DDR_TIMING_0		0x00260802
#define CONFIG_SYS_DDR_TIMING_1		0x3837c322
#define CONFIG_SYS_DDR_TIMING_2		0x0f9848c6
#define CONFIG_SYS_DDR_TIMING_3		0x00000000

#define CONFIG_SYS_DDR_SDRAM_CFG	0x43080008
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#define CONFIG_SYS_DDR_INTERVAL		0x02000100

#define CONFIG_SYS_DDR_MODE		0x04040242
#define CONFIG_SYS_DDR_MODE2		0x00800000

/* Flash */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_BASE		0xFF800000
#define CONFIG_SYS_FLASH_SIZE		8
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE \
				| BR_PS_16 \
				| BR_MS_GPCM \
				| BR_V)
#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
				| OR_UPM_XAM \
				| OR_GPCM_CSNT \
				| OR_GPCM_ACS_DIV2 \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_15 \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_8MB)

/*
 * U-Boot memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#undef	CONFIG_SYS_RAMBOOT

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)

/*
 * Local Bus LCRR and LBCR regs
 *  LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *  CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4
#define CONFIG_SYS_LBC_LBCR	0x00000000

/* LB sdram refresh timer, about 6us */
#define CONFIG_SYS_LBC_LSRT	0x32000000
/* LB refresh timer prescal, 266MHz/32*/
#define CONFIG_SYS_LBC_MRTPR	0x20000000

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_CONSOLE		ttyS0
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1
#define MV_DTB_NAME	"mvblm7.dtb"

/*
 * PCI
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCI1_MMIO_BASE	\
			(CONFIG_SYS_PCI1_MEM_BASE + CONFIG_SYS_PCI1_MEM_SIZE)
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000
#define CONFIG_SYS_PCI1_IO_BASE	0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS	0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE	0x01000000

#define CONFIG_NET_RETRY_COUNT	3

#define CONFIG_PCI_66M
#define CONFIG_83XX_CLKIN	66666667
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW

/* TSEC */
#define CONFIG_GMII
#define CONFIG_SYS_VSC8601_SKEWFIX
#define	CONFIG_SYS_VSC8601_SKEW_TX	3
#define	CONFIG_SYS_VSC8601_SKEW_RX	3

#define CONFIG_TSEC1
#define CONFIG_TSEC2

#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_FEC1_PHY_NORXERR
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define TSEC1_PHY_ADDR		0x10
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT|TSEC_REDUCED)

#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_FEC2_PHY_NORXERR
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)
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
#define CONFIG_SYS_USB_HOST
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

/*
 * Environment
 */
#undef  CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_ENV_OVERWRITE

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0xFF800000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR+CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_LOADS_ECHO
#define CONFIG_SYS_LOADS_BAUD_CHANGE

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
#define CONFIG_CMD_USB
#define CONFIG_DOS_PARTITION

#undef CONFIG_WATCHDOG

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE	/* add autocompletion support   */
#define CONFIG_SYS_HUSH_PARSER

/* default load address */
#define CONFIG_SYS_LOAD_ADDR	0x2000000
/* default location for tftp and bootm */
#define CONFIG_LOADADDR	0x200000

#define CONFIG_SYS_PROMPT	"mvBL-M7> "
#define CONFIG_SYS_CBSIZE	256

#define CONFIG_SYS_PBSIZE	\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_HRCW_LOW	0x0
#define CONFIG_SYS_HRCW_HIGH	0x0

/*
 * System performance
 */
#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT	3	/* Arbiter repeat count (0-7) */
#define CONFIG_SYS_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CONFIG_SYS_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */

/* clocking */
#define CONFIG_SYS_SCCR_ENCCM		0
#define CONFIG_SYS_SCCR_USBMPHCM	0
#define	CONFIG_SYS_SCCR_USBDRCM	2
#define CONFIG_SYS_SCCR_TSEC1CM	1
#define CONFIG_SYS_SCCR_TSEC2CM	1

#define CONFIG_SYS_SICRH	0x1fef0003
#define CONFIG_SYS_SICRL	(SICRL_LDP_A | SICRL_USB1 | SICRL_USB0)

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(CONFIG_SYS_HID0_INIT | \
				 HID0_ENABLE_INSTRUCTION_CACHE)

#define CONFIG_SYS_HID2	HID2_HBE
#define CONFIG_HIGH_BATS	1

/* DDR  */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI  */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* no PCI2 */
#define CONFIG_SYS_IBAT3L	0
#define CONFIG_SYS_IBAT3U	0
#define CONFIG_SYS_IBAT4L	0
#define CONFIG_SYS_IBAT4U	0

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_IMMR \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* stack in DCACHE 0xFDF00000 & FLASH @ 0xFF800000 */
#define CONFIG_SYS_IBAT6L	(0xF0000000 \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(0xF0000000 \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT7L	0
#define CONFIG_SYS_IBAT7U	0

#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

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
#define MV_FPGA_DATA		0xfff40000
#define MV_FPGA_SIZE		0
#define MV_KERNEL_ADDR		0xff810000
#define MV_INITRD_ADDR		0xffb00000
#define MV_SCRIPT_ADDR		0xff804000
#define MV_SCRIPT_ADDR2		0xff806000
#define MV_DTB_ADDR		0xff808000
#define MV_INITRD_LENGTH	0x00400000

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define MV_KERNEL_ADDR_RAM	0x00100000
#define MV_DTB_ADDR_RAM		0x00600000
#define MV_INITRD_ADDR_RAM	0x01000000

#define CONFIG_BOOTCOMMAND	"if imi ${script_addr}; "		\
					"then source ${script_addr}; "	\
					"else source ${script_addr2}; "	\
				"fi;"
#define CONFIG_BOOTARGS		"root=/dev/ram ro rootfstype=squashfs"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"console_nr=0\0"					\
	"baudrate=" __stringify(CONFIG_BAUDRATE) "\0"		\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"fpga=0\0"						\
	"fpgadata=" __stringify(MV_FPGA_DATA) "\0"			\
	"fpgadatasize=" __stringify(MV_FPGA_SIZE) "\0"		\
	"script_addr=" __stringify(MV_SCRIPT_ADDR) "\0"		\
	"script_addr2=" __stringify(MV_SCRIPT_ADDR2) "\0"		\
	"mv_kernel_addr=" __stringify(MV_KERNEL_ADDR) "\0"		\
	"mv_kernel_addr_ram=" __stringify(MV_KERNEL_ADDR_RAM) "\0"	\
	"mv_initrd_addr=" __stringify(MV_INITRD_ADDR) "\0"		\
	"mv_initrd_addr_ram=" __stringify(MV_INITRD_ADDR_RAM) "\0"	\
	"mv_initrd_length=" __stringify(MV_INITRD_LENGTH) "\0"	\
	"mv_dtb_addr=" __stringify(MV_DTB_ADDR) "\0"			\
	"mv_dtb_addr_ram=" __stringify(MV_DTB_ADDR_RAM) "\0"		\
	"dtb_name=" __stringify(MV_DTB_NAME) "\0"			\
	"mv_version=" U_BOOT_VERSION "\0"			\
	"dhcp_client_id=" MV_CI "\0"				\
	"dhcp_vendor-class-identifier=" MV_VCI "\0"		\
	"netretry=no\0"						\
	"use_static_ipaddr=no\0"				\
	"static_ipaddr=192.168.90.10\0"				\
	"static_netmask=255.255.255.0\0"			\
	"static_gateway=0.0.0.0\0"				\
	"initrd_name=uInitrd.mvBL-M7-rfs\0"			\
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
	"sensor_cnt=2\0"					\
	""

#define CONFIG_FPGA_COUNT	1
#define CONFIG_FPGA
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_CYCLON2

#endif
