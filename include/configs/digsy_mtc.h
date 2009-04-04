/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005-2007
 * Modified for InterControl digsyMTC MPC5200 board by
 * Frank Bodammer, GCD Hard- & Software GmbH,
 *                 frank.bodammer@gcd-solutions.de
 *
 * (C) Copyright 2009 Semihalf
 * Optimized for digsyMTC by: Grzegorz Bernacki <gjb@semihalf.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software\; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation\; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY\; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program\; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU) */
#define CONFIG_DIGSY_MTC	1	/* ... on InterControl digsyMTC board */

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000

#define BOOTFLAG_COLD		0x01
#define BOOTFLAG_WARM		0x02

#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	4	/* console is on PSC4  */
#define CONFIG_BAUDRATE		115200	/* ... at 115200  bps  */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

/*
 *  Partitions
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_BZIP2

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DFL
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_USB

#if (TEXT_BASE == 0xFF000000)
#define CONFIG_SYS_LOWBOOT	1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	1

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS			\
	"netdev=eth0\0"					\
	"console=ttyPSC0\0"				\
	"kernel_addr_r=400000\0"			\
	"fdt_addr_r=600000\0"				\
	"nfsargs=setenv bootargs root=/dev/nfs rw "	\
	"nfsroot=${serverip}:${rootpath}\0"		\
	"addip=setenv bootargs ${bootargs} "		\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"\
		"${netmask}:${hostname}:${netdev}:off panic=1\0"	\
	"addcons=setenv bootargs ${bootargs} console=${console},${baudrate}\0"\
	"rootpath=/opt/eldk/ppc_6xx\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"	\
		"tftp ${fdt_addr_r} ${fdt_file};"	\
		"run nfsargs addip addcons;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"	\
	"load=tftp 200000 ${u-boot}\0"			\
	"update=protect off FFF00000 +${filesize};"	\
		"erase FFF00000 +${filesize};"		\
		"cp.b 200000 FFF00000 ${filesize};"	\
		"protect on FFF00000 +${filesize}\0"	\
	""

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_MODULE	1
#define CONFIG_SYS_I2C_SPEED	100000
#define CONFIG_SYS_I2C_SLAVE	0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * RTC configuration
 */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR	0x68

/*
 * Flash configuration
 */
#define	CONFIG_SYS_FLASH_CFI		1
#define	CONFIG_FLASH_CFI_DRIVER	1

#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_SIZE	0x01000000

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_FLASH_16BIT
#define CONFIG_SYS_FLASH_CFI_WIDTH FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_FLASH_ERASE_TOUT	240000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

#define CONFIG_OF_LIBFDT  1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#if defined(CONFIG_LOWBOOT)
#define CONFIG_ENV_ADDR		0xFF060000
#else	/* CONFIG_LOWBOOT */
#define CONFIG_ENV_ADDR		0xFFF60000
#endif	/* CONFIG_LOWBOOT */
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000
#else
#define CONFIG_SYS_DEFAULT_MBAR	0xF0000000
#endif

/*
 *  Use SRAM until RAM will be available
 */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_SIZE	4096
#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN	(256 << 10)
#define CONFIG_SYS_MALLOC_LEN	(4096 << 10)
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_PHY_RESET_DELAY	1000

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0xA2552112

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_SYS_PROMPT	"=> "
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT "autoboot in %d seconds\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	" "

#define CONFIG_LOOPW		1
#define CONFIG_MX_CYCLIC	1
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_SCRATCH	0x00001000
#define CONFIG_SYS_MEMTEST_START	0x00010000
#define CONFIG_SYS_MEMTEST_END		0x019fffff

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define CONFIG_SYS_HZ			1000

/*
 * Various low-level settings
 */
#define CONFIG_SYS_SDRAM_CS1		1
#define CONFIG_SYS_XLB_PIPELINING	1

#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#if defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x0002DD00
#endif

#define CONFIG_SYS_CS4_START		0x60000000
#define CONFIG_SYS_CS4_SIZE		0x1000
#define CONFIG_SYS_CS4_CFG		0x0008FC00

#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS0_CFG		0x0002DD00

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x11111111

#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#else
#define CONFIG_SYS_RESET_ADDRESS	0xff000100
#endif

/*
 * USB
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#define CONFIG_USB_STORAGE

#define CONFIG_USB_CLOCK	0x00013333
#define CONFIG_USB_CONFIG	0x00002000

#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_SYS_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"mpc5200"
#define CONFIG_SYS_USB_OHCI_CPU_INIT

/*
 * IDE/ATA
 */
#define CONFIG_IDE_RESET
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_ATA_CS_ON_I2C2
#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	1

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)
#define CONFIG_SYS_ATA_STRIDE		4

#define CONFIG_ATAPI		1
#define CONFIG_LBA48		1

#endif /* __CONFIG_H */
