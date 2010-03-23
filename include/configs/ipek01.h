/*
 * (C) Copyright 2006
 * MicroSys GmbH
 *
 * (C) Copyright 2009
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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

/*
 * High Level Configuration Options
 */

#define CONFIG_MPC5200
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPX5200		1	/* ... on MPX5200 board */
#define CONFIG_MPC5200_DDR	1	/* ... use DDR RAM */
#define CONFIG_IPEK01           	/* Motherboard is ipek01 */

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33MHz */

#define CONFIG_MISC_INIT_R

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

#define CONFIG_SYS_CACHELINE_SIZE	32 /* For MPC5xxx CPUs */
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5  /* log base 2 of the above value */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 9600 bps */
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */

/*
 * Video configuration for LIME GDC
 */
#define CONFIG_VIDEO
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_MB862xx
#define CONFIG_VIDEO_MB862xx_ACCEL
#define VIDEO_FB_16BPP_WORD_SWAP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_SPLASH_SCREEN
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)	/* decompressed img */
/* Lime clock frequency */
#define CONFIG_SYS_MB862xx_CCF	0x90000	/* geo 166MHz other 133MHz */
/* SDRAM parameter */
#define CONFIG_SYS_MB862xx_MMR	0x41c767e3
#endif

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

#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */

/* Partitions */
#define CONFIG_DOS_PARTITION

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#define CONFIG_USB_STORAGE

#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"mpc5200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#ifdef CONFIG_VIDEO
#define CONFIG_CMD_BMP		/* BMP support */
#endif
#define CONFIG_CMD_DATE		/* support for RTC, date/time...*/
#define CONFIG_CMD_DHCP		/* DHCP Support */
#define CONFIG_CMD_FAT		/* FAT support */
#define CONFIG_CMD_I2C		/* I2C serial bus support */
#define CONFIG_CMD_IDE		/* IDE harddisk support */
#define CONFIG_CMD_IRQ		/* irqinfo */
#define CONFIG_CMD_MII		/* MII support */
#define CONFIG_CMD_PCI		/* pciinfo */
#define CONFIG_CMD_USB		/* USB Support */

#define CONFIG_SYS_LOWBOOT	1

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyPSC0\0"						\
	"hostname=ipek01\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} "				\
		"console=${consoledev},${baudrate}\0"			\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr} - ${fdtaddr}\0"			\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdtaddr}\0"	\
	"net_nfs=tftp 200000 ${bootfile}; tftp ${fdtaddr} ${fdtfile};"  \
		"run nfsargs addip addtty;"    				\
		 "bootm ${loadaddr} - ${fdtaddr}\0"			\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=ipek01/uImage\0"					\
	"load=tftp 100000 ipek01/u-boot.bin\0"				\
	"update=protect off FC000000 +60000; era FC000000 +60000; "	\
		"cp.b 100000 FC000000 ${filesize}\0"   			\
	"upd=run load;run update\0"					\
	"fdtaddr=800000\0"						\
	"loadaddr=400000\0"						\
	"fdtfile=ipek01/ipek01.dtb\0"					\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK 	/* for 133MHz */
/* PCI clock must be 33, because board will not boot */
#undef CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2	/* for 66MHz */

/*
 * Open firmware flat tree support
 */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE	2	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED	100000	/* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE	0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x53
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + \
					 CONFIG_SYS_MONITOR_LEN)

#define CONFIG_SYS_MAX_FLASH_BANKS	1    /* max num of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256  /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_PROTECTION  /* "Real" (hardware) sectors protection */

/* use CFI flash driver */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_OVERWRITE		1
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR			0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR		0x80000000
#define	CONFIG_SYS_SRAM_BASE		0xF1000000
#define	CONFIG_SYS_SRAM_SIZE		0x00200000
#define	CONFIG_SYS_LIME_BASE		0xE4000000
#define	CONFIG_SYS_LIME_SIZE		0x04000000
#define	CONFIG_SYS_FPGA_BASE		0xC0000000
#define	CONFIG_SYS_FPGA_SIZE		0x10000000
#define	CONFIG_SYS_MPEG_BASE		0xe2000000
#define	CONFIG_SYS_MPEG_SIZE		0x01000000
#define CONFIG_SYS_CF_BASE		0xe1000000
#define CONFIG_SYS_CF_SIZE		0x01000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
/* End of used area in DPRAM */
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_SIZE

/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN	(384 << 10)  /* Reserve 384 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN	(4 << 20)    /* Reserve 128 kB for malloc() */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)    /* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC		1
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR			0x00

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x1d556624

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt */
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS		16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1...15 MB in DRAM */

#define CONFIG_SYS_LOAD_ADDR		0x100000 /* default load address */

#define CONFIG_SYS_HZ			1000 /* decrementer freq: 1 ms ticks */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE
#else
#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		0
#endif

#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS1_START		CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_SRAM_SIZE
#define CONFIG_SYS_CS3_START		CONFIG_SYS_LIME_BASE
#define CONFIG_SYS_CS3_SIZE		CONFIG_SYS_LIME_SIZE
#define	CONFIG_SYS_CS6_START		CONFIG_SYS_FPGA_BASE
#define	CONFIG_SYS_CS6_SIZE		CONFIG_SYS_FPGA_SIZE
#define	CONFIG_SYS_CS5_START		CONFIG_SYS_CF_BASE
#define	CONFIG_SYS_CS5_SIZE		CONFIG_SYS_CF_SIZE
#define	CONFIG_SYS_CS7_START		CONFIG_SYS_MPEG_BASE
#define	CONFIG_SYS_CS7_SIZE		CONFIG_SYS_MPEG_SIZE

#ifdef CONFIG_SYS_PCISPEED_66
#define CONFIG_SYS_BOOTCS_CFG		0x0006F900
#define CONFIG_SYS_CS1_CFG		0x0004FB00
#define CONFIG_SYS_CS2_CFG		0x0006F900
#else
#define CONFIG_SYS_BOOTCS_CFG		0x0002F900
#define CONFIG_SYS_CS1_CFG		0x0001FB00
#define CONFIG_SYS_CS2_CFG		0x0002F90C
#endif

/*
 * Ack active, Muxed mode, AS=24 bit address, DS=32 bit data, 0
 * waitstates, writeswap and readswap enabled
 */
#define CONFIG_SYS_CS3_CFG		0x00FFFB0C
#define	CONFIG_SYS_CS6_CFG		0x00FFFB0C
#define	CONFIG_SYS_CS7_CFG		0x4040751C

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE		0x33330000

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK		0x0001BBBB
#define CONFIG_USB_CONFIG		0x00005000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1 /* max. 1 IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	2 /* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O */
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses */
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers */
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers */
#define CONFIG_SYS_ATA_STRIDE		4

#endif /* __CONFIG_H */
