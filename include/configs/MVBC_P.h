/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2008
 * Matrix-Vision GmbH, andre.schwarz@matrix-vision.de
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

#define CONFIG_MPC5xxx	1
#define CONFIG_MPC5200 	1

#define CFG_MPC5XXX_CLKIN	33000000

#define BOOTFLAG_COLD		0x01
#define BOOTFLAG_WARM		0x02

#define CONFIG_MISC_INIT_R	1

#define CFG_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CFG_CACHELINE_SHIFT	5
#endif

#define CONFIG_PSC_CONSOLE	1
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200, 230400}

#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#undef	CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CFG_XLB_PIPELINING	1
#define CONFIG_HIGH_BATS	1

#define MV_CI			mvBlueCOUGAR-P
#define MV_VCI			mvBlueCOUGAR-P
#define MV_FPGA_DATA		0xff860000
#define MV_FPGA_SIZE		0x0003c886
#define MV_KERNEL_ADDR		0xffc00000
#define MV_INITRD_ADDR		0xff900000
#define MV_INITRD_LENGTH	0x00300000
#define MV_SCRATCH_ADDR		0x00000000
#define MV_SCRATCH_LENGTH	MV_INITRD_LENGTH
#define MV_AUTOSCR_ADDR		0xff840000
#define MV_AUTOSCR_ADDR2	0xff850000
#define MV_DTB_ADDR		0xfffc0000

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define MV_KERNEL_ADDR_RAM	0x00100000
#define MV_DTB_ADDR_RAM		0x00600000
#define MV_INITRD_ADDR_RAM	0x01000000

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define MV_DTB_NAME		mvbc-p.dtb
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * Supported commands
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_PCI
#define CONFIG_CMD_FPGA

#undef CONFIG_WATCHDOG

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

/*
 * Autoboot
 */
#define CONFIG_BOOTDELAY		2
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR 	"s"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY		1000

#define CONFIG_BOOTCOMMAND	"if imi ${autoscr_addr}; \
					then autoscr ${autoscr_addr};	\
					else autoscr ${autoscr_addr2};	\
				fi;"

#define CONFIG_BOOTARGS		"root=/dev/ram ro rootfstype=squashfs"
#define CONFIG_ENV_OVERWRITE

#define XMK_STR(x)      #x
#define MK_STR(x)       XMK_STR(x)

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"console_nr=0\0"					\
	"console=yes\0"						\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"fpga=0\0"						\
	"fpgadata=" MK_STR(MV_FPGA_DATA) "\0"			\
	"fpgadatasize=" MK_STR(MV_FPGA_SIZE) "\0"		\
	"autoscr_addr=" MK_STR(MV_AUTOSCR_ADDR) "\0"		\
	"autoscr_addr2=" MK_STR(MV_AUTOSCR_ADDR2) "\0"		\
	"mv_kernel_addr=" MK_STR(MV_KERNEL_ADDR) "\0"		\
	"mv_kernel_addr_ram=" MK_STR(MV_KERNEL_ADDR_RAM) "\0"	\
	"mv_initrd_addr=" MK_STR(MV_INITRD_ADDR) "\0"		\
	"mv_initrd_addr_ram=" MK_STR(MV_INITRD_ADDR_RAM) "\0"	\
	"mv_initrd_length=" MK_STR(MV_INITRD_LENGTH) "\0"	\
	"mv_dtb_addr=" MK_STR(MV_DTB_ADDR) "\0"			\
	"mv_dtb_addr_ram=" MK_STR(MV_DTB_ADDR_RAM) "\0"		\
	"dtb_name=" MK_STR(MV_DTB_NAME) "\0"			\
	"mv_scratch_addr=" MK_STR(MV_SCRATCH_ADDR) "\0"		\
	"mv_scratch_length=" MK_STR(MV_SCRATCH_LENGTH) "\0"	\
	"mv_version=" U_BOOT_VERSION "\0"			\
	"dhcp_client_id=" MK_STR(MV_CI) "\0"			\
	"dhcp_vendor-class-identifier=" MK_STR(MV_VCI) "\0"	\
	"netretry=no\0"						\
	"use_static_ipaddr=no\0"				\
	"static_ipaddr=192.168.90.10\0"				\
	"static_netmask=255.255.255.0\0"			\
	"static_gateway=0.0.0.0\0"				\
	"initrd_name=uInitrd.mvbc-p-rfs\0"			\
	"zcip=no\0"						\
	"netboot=yes\0"						\
	"mvtest=Ff\0"						\
	"tried_bootfromflash=no\0"				\
	"tried_bootfromnet=no\0"				\
	"use_dhcp=yes\0"					\
	"gev_start=yes\0"					\
	"mvbcdma_debug=0\0"					\
	"mvbcia_debug=0\0"					\
	"propdev_debug=0\0"					\
	"gevss_debug=0\0"					\
	"watchdog=1\0"						\
	""

#undef XMK_STR
#undef MK_STR

/*
 * IPB Bus clocking configuration.
 */
#define CFG_IPBCLK_EQUALS_XLBCLK
#define CFG_PCICLK_EQUALS_IPBCLK_DIV2

/*
 * Flash configuration
 */
#undef 	CONFIG_FLASH_16BIT
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI_AMD_RESET 1
#define CFG_FLASH_EMPTY_INFO

#define CFG_FLASH_ERASE_TOUT	50000
#define CFG_FLASH_WRITE_TOUT	1000

#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	256

#define CFG_LOWBOOT
#define CFG_FLASH_BASE		TEXT_BASE
#define CFG_FLASH_SIZE		0x00800000

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH
#undef	CFG_FLASH_PROTECTION

#define CFG_ENV_ADDR		0xFFFE0000
#define CFG_ENV_SIZE		0x10000
#define CFG_ENV_SECT_SIZE	0x10000
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	CFG_ENV_SIZE

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE

#define CFG_GBL_DATA_SIZE	128
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT		1
#endif

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(512 << 10)
#define CFG_MALLOC_LEN		(512 << 10)
#define CFG_BOOTMAPSZ		(8 << 20)

/*
 * Ethernet configuration
 */
#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT 5

#define CONFIG_E1000
#define CONFIG_E1000_FALLBACK_MAC	{ 0xb6, 0xb4, 0x45, 0xeb, 0xfb, 0xc0 }
#undef CONFIG_MPC5xxx_FEC
#undef CONFIG_PHY_ADDR
#define CONFIG_NETDEV		eth0

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CFG_PROMPT_HUSH_PS2 	"> "
#undef 	CFG_LONGHELP
#define CFG_PROMPT		"=> "
#ifdef CONFIG_CMD_KGDB
#define CFG_CBSIZE		1024
#else
#define CFG_CBSIZE		256
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		16
#define CFG_BARGSIZE		CFG_CBSIZE

#define CFG_MEMTEST_START	0x00800000
#define CFG_MEMTEST_END		0x02f00000

#define CFG_HZ			1000

/* default load address */
#define CFG_LOAD_ADDR		0x02000000
/* default location for tftp and bootm */
#define CONFIG_LOADADDR 	0x00200000

/*
 * Various low-level settings
 */
#define CFG_GPS_PORT_CONFIG	0x20000004

#define CFG_HID0_INIT		(HID0_ICE | HID0_ICFI)
#define CFG_HID0_FINAL		HID0_ICE

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047800
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

#define CFG_CS_BURST		0x000000f0
#define CFG_CS_DEADCYCLE	0x33333303

#define CFG_RESET_ADDRESS	0x00000100

#undef FPGA_DEBUG
#undef CFG_FPGA_PROG_FEEDBACK
#define CONFIG_FPGA		CFG_ALTERA_CYCLON2
#define CONFIG_FPGA_ALTERA	1
#define CONFIG_FPGA_CYCLON2	1
#define CONFIG_FPGA_COUNT	1

#endif
