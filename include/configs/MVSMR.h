/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2010
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

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000

#define BOOTFLAG_COLD		0x01
#define BOOTFLAG_WARM		0x02

#define CONFIG_MISC_INIT_R	1

#define CONFIG_SYS_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5
#endif

#define CONFIG_PSC_CONSOLE	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200,\
						230400}

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

#define CONFIG_SYS_XLB_PIPELINING	1
#define CONFIG_HIGH_BATS	1

#define MV_CI			mvSMR
#define MV_VCI			mvSMR
#define MV_FPGA_DATA		0xff840000
#define MV_FPGA_SIZE		0x1ff88
#define MV_KERNEL_ADDR		0xfff00000
#define MV_SCRIPT_ADDR		0xff806000
#define MV_INITRD_ADDR		0xff880000
#define MV_INITRD_LENGTH	0x00240000
#define MV_SCRATCH_ADDR		0xffcc0000
#define MV_SCRATCH_LENGTH	MV_INITRD_LENGTH

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define MV_KERNEL_ADDR_RAM	0x00100000
#define MV_INITRD_ADDR_RAM	0x00400000

/*
 * Supported commands
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FPGA
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SDRAM

#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_RANDOM_DELAY
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_VENDOREX

/*
 * Autoboot
 */
#define CONFIG_BOOTDELAY		1
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR 	"abcdefg"
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_BOOTCOMMAND      "source ${script_addr}"
#define CONFIG_BOOTARGS		"root=/dev/ram ro rootfstype=squashfs" \
					" allocate=6M"

#define XMK_STR(x)      #x
#define MK_STR(x)       XMK_STR(x)

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"console_nr=0\0"					\
	"console=no\0"						\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"fpga=0\0"						\
	"fpgadata=" MK_STR(MV_FPGA_DATA) "\0"			\
	"fpgadatasize=" MK_STR(MV_FPGA_SIZE) "\0"		\
	"mv_kernel_addr=" MK_STR(MV_KERNEL_ADDR) "\0"		\
	"mv_kernel_addr_ram=" MK_STR(MV_KERNEL_ADDR_RAM) "\0"	\
	"script_addr=" MK_STR(MV_SCRIPT_ADDR) "\0"		\
	"mv_initrd_addr=" MK_STR(MV_INITRD_ADDR) "\0"		\
	"mv_initrd_addr_ram=" MK_STR(MV_INITRD_ADDR_RAM) "\0"	\
	"mv_initrd_length=" MK_STR(MV_INITRD_LENGTH) "\0"	\
	"mv_scratch_addr=" MK_STR(MV_SCRATCH_ADDR) "\0"		\
	"mv_scratch_length=" MK_STR(MV_SCRATCH_LENGTH) "\0"	\
	"mv_version=" U_BOOT_VERSION "\0"			\
	"dhcp_client_id=" MK_STR(MV_CI) "\0"			\
	"dhcp_vendor-class-identifier=" MK_STR(MV_VCI) "\0"	\
	"netretry=no\0"						\
	"use_static_ipaddr=no\0"				\
	"static_ipaddr=192.168.0.101\0"				\
	"static_netmask=255.255.255.0\0"			\
	"static_gateway=0.0.0.0\0"				\
	"initrd_name=uInitrd.mvsmr-rfs\0"			\
	"zcip=yes\0"						\
	"netboot=no\0"						\
	""

#undef XMK_STR
#undef MK_STR

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK

/*
 * Flash configuration
 */
#undef 	CONFIG_FLASH_16BIT
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_AMD_RESET 1
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_FLASH_ERASE_TOUT	50000
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_SYS_LOWBOOT
#define CONFIG_SYS_FLASH_BASE		TEXT_BASE
#define CONFIG_SYS_FLASH_SIZE		0x00800000

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH
#undef	CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_HAS_UID
#define	CONFIG_OVERWRITE_ETHADDR_ONCE

#define CONFIG_ENV_OFFSET	0x8000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000

/* used by linker script to wrap code around */
#define CONFIG_SCRIPT_OFFSET	0x6000
#define CONFIG_SCRIPT_SECT_SIZE	0x2000

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE	0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
						CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT		1
#endif

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)
#define CONFIG_SYS_MALLOC_LEN		(512 << 10)
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_MODULE	1
#define CONFIG_SYS_I2C_SPEED	86000
#define CONFIG_SYS_I2C_SLAVE	0x7F

/*
 * Ethernet configuration
 */
#define CONFIG_NET_RETRY_COUNT 5

#define CONFIG_MPC5xxx_FEC
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_NETDEV		eth0

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_PROMPT_HUSH_PS2 	"> "
#undef 	CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"=> "
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CBSIZE		1024
#else
#define CONFIG_SYS_CBSIZE		256
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x00800000
#define CONFIG_SYS_MEMTEST_END		0x02f00000

#define CONFIG_SYS_HZ			1000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0x02000000
/* default location for tftp and bootm */
#define CONFIG_LOADADDR 		0x00200000

/*
 * Various low-level settings
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x00050044

#define CONFIG_SYS_HID0_INIT		(HID0_ICE | HID0_ICFI)
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00047800
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_SYS_CS_BURST		0x000000f0
#define CONFIG_SYS_CS_DEADCYCLE		0x33333303

#define CONFIG_SYS_RESET_ADDRESS	0x00000100

#undef FPGA_DEBUG
#undef CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_FPGA		CONFIG_SYS_XILINX_SPARTAN2
#define CONFIG_FPGA_XILINX	1
#define CONFIG_FPGA_SPARTAN2	1
#define CONFIG_FPGA_COUNT	1

#endif
