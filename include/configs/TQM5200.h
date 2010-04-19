/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2006
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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
 * (easy to change)
 */

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU		*/
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU)	*/
#define CONFIG_TQM5200		1	/* ... on TQM5200 module		*/
#undef CONFIG_TQM5200_REV100		/*  define for revision 100 modules	*/

/* On a Cameron or on a FO300 board or ...				*/
#if !defined(CONFIG_CAM5200) && !defined(CONFIG_FO300)
#define CONFIG_STK52XX		1	/* ... on a STK52XX board		*/
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz		*/

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define CONFIG_HIGH_BATS	1	/* High BATs supported			*/

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1			*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps			*/
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }
#define CONFIG_BOOTCOUNT_LIMIT	1

#ifdef CONFIG_FO300
#define CONFIG_SYS_DEVICE_NULLDEV		1	/* enable null device */
#define CONFIG_SILENT_CONSOLE		1	/* enable silent startup */
#define CONFIG_BOARD_EARLY_INIT_F	1	/* used to detect S1 switch position */
#define CONFIG_USB_BIN_FIXUP		1	/* for a buggy USB device */
#if 0
#define FO300_SILENT_CONSOLE_WHEN_S1_CLOSED	1	/* silent console on PSC1 when S1 */
							/* switch is closed */
#endif

#undef FO300_SILENT_CONSOLE_WHEN_S1_CLOSED		/* silent console on PSC1 when S1 */
							/* switch is open */
#endif	/* CONFIG_FO300 */

#ifdef CONFIG_STK52XX
#define CONFIG_PS2KBD			/* AT-PS/2 Keyboard		*/
#define CONFIG_PS2MULT			/* .. on PS/2 Multiplexer	*/
#define CONFIG_PS2SERIAL	6	/* .. on PSC6			*/
#define CONFIG_PS2MULT_DELAY	(CONFIG_SYS_HZ/2)	/* Initial delay	*/
#define CONFIG_BOARD_EARLY_INIT_R
#endif /* CONFIG_STK52XX */

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#ifdef CONFIG_STK52XX
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
/* #define CONFIG_PCI_SCAN_SHOW	1 */

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_NET_MULTI	1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1
#endif	/* CONFIG_STK52XX */

/*
 * Video console
 */
#ifndef CONFIG_TQM5200S		/* No graphics controller on TQM5200S */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_SM501
#define CONFIG_VIDEO_SM501_32BPP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO

#ifndef CONFIG_FO300
#define CONFIG_CONSOLE_EXTRA_INFO
#else
#define CONFIG_VIDEO_BMP_LOGO
#endif

#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#endif /* #ifndef CONFIG_TQM5200S */


/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/* USB */
#if defined(CONFIG_STK52XX) || defined(CONFIG_FO300)
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"mpc5200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

#endif

#ifndef CONFIG_CAM5200
/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_MEMORY   | \
				 CONFIG_SYS_POST_CPU	   | \
				 CONFIG_SYS_POST_I2C)
#endif

#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define MPC5XXX_SRAM_POST_SIZE MPC5XXX_SRAM_SIZE-4
#endif


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_BSP

#ifdef CONFIG_VIDEO
    #define CONFIG_CMD_BMP
#endif

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1
#endif

#if defined(CONFIG_MINIFAP) || defined(CONFIG_STK52XX) || defined(CONFIG_FO300)
    #define CONFIG_CMD_IDE
    #define CONFIG_CMD_FAT
    #define CONFIG_CMD_EXT2
#endif

#if defined(CONFIG_STK52XX) || defined(CONFIG_FO300)
    #define CONFIG_CFG_USB
    #define CONFIG_CFG_FAT
#endif

#ifdef CONFIG_POST
    #define CONFIG_CMD_DIAG
#endif


#define	CONFIG_TIMESTAMP		/* display image timestamps */

#if (TEXT_BASE != 0xFFF00000)
#   define CONFIG_SYS_LOWBOOT		1	/* Boot low */
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#if defined(CONFIG_TQM5200_B) && !defined(CONFIG_SYS_LOWBOOT)
# define ENV_UPDT							\
	"update=protect off FFF00000 +${filesize};"			\
		"erase FFF00000 +${filesize};"				\
		"cp.b 200000 FFF00000 ${filesize};"			\
		"protect on FFF00000 +${filesize}\0"
#else	/* default lowboot configuration */
#   define ENV_UPDT							\
	"update=protect off FC000000 +${filesize};"			\
		"erase FC000000 +${filesize};"				\
		"cp.b 200000 FC000000 ${filesize};"			\
		"protect on FC000000 +${filesize}\0"
#endif

#if defined(CONFIG_TQM5200)
#define CUSTOM_ENV_SETTINGS						\
	"hostname=tqm5200\0"						\
	"bootfile=/tftpboot/tqm5200/uImage\0"				\
	"fdt_file=/tftpboot/tqm5200/tqm5200.dtb\0"			\
	"u-boot=/tftpboot/tqm5200/u-boot.bin\0"
#elif defined(CONFIG_CAM5200)
#define CUSTOM_ENV_SETTINGS						\
	"bootfile=cam5200/uImage\0"					\
	"u-boot=cam5200/u-boot.bin\0"					\
	"setup=tftp 200000 cam5200/setup.img; source 200000\0"
#endif

#if defined(CONFIG_TQM5200_B)
#define ENV_FLASH_LAYOUT						\
	"fdt_addr=FC100000\0"						\
	"kernel_addr=FC140000\0"					\
	"ramdisk_addr=FC600000\0"
#else	/* !CONFIG_TQM5200_B */
#define ENV_FLASH_LAYOUT						\
	"fdt_addr=FC0A0000\0"						\
	"kernel_addr=FC0C0000\0"					\
	"ramdisk_addr=FC300000\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"console=ttyPSC0\0"						\
	ENV_FLASH_LAYOUT						\
	"kernel_addr_r=400000\0"					\
	"fdt_addr_r=600000\0"						\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console},${baudrate}\0"			\
	"flash_self_old=sete console ttyS0; run ramargs addip addcons;"	\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_self=run ramargs addip addcons;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"flash_nfs_old=sete console ttyS0; run nfsargs addip addcons;"	\
		"bootm ${kernel_addr}\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_nfs_old=tftp ${kernel_addr_r} ${bootfile};"		\
		"sete console ttyS0; run nfsargs addip addcons;bootm\0"	\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addcons; "				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	CUSTOM_ENV_SETTINGS						\
	"load=tftp 200000 ${u-boot}\0"					\
	ENV_UPDT							\
	""

#define CONFIG_BOOTCOMMAND	"run net_nfs"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

#if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK) && !defined(CONFIG_CAM5200)
/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CONFIG_SYS_IPBCLK_EQUALS_XLBCLK is defined. This is because a PCI Clock of
 * 66 MHz yet hasn't been tested with a IPB Bus Clock of 66 MHz.
 */
#define CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2	/* define for 66MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#ifdef CONFIG_TQM5200_REV100
#define CONFIG_SYS_I2C_MODULE		1	/* Select I2C module #1 for rev. 100 board */
#else
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #2 for all other revs */
#endif

/*
 * I2C clock frequency
 *
 * Please notice, that the resulting clock frequency could differ from the
 * configured value. This is because the I2C clock is derived from system
 * clock over a frequency divider with only a few divider values. U-boot
 * calculates the best approximation for CONFIG_SYS_I2C_SPEED. However the calculated
 * approximation allways lies below the configured value, never above.
 */
#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration for onboard EEPROM M24C32 (M24C64 should work
 * also). For other EEPROMs configuration should be verified. On Mini-FAP the
 * EEPROM (24C64) is on the same I2C address (but on other I2C bus), so the
 * same configuration could be used.
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* =32 Bytes per write */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20

/*
 * HW-Monitor configuration on Mini-FAP
 */
#if defined (CONFIG_MINIFAP)
#define CONFIG_SYS_I2C_HWMON_ADDR		0x2C
#endif

/* List of I2C addresses to be verified by POST */
#if defined (CONFIG_MINIFAP)
#undef I2C_ADDR_LIST
#define I2C_ADDR_LIST	{	CONFIG_SYS_I2C_EEPROM_ADDR,	\
				CONFIG_SYS_I2C_HWMON_ADDR,	\
				CONFIG_SYS_I2C_SLAVE }
#endif

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xFC000000

#if defined(CONFIG_CAM5200) && defined(CONFIG_CAM5200_NIOSFLASH)
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of flash banks
					   (= chip selects) */
#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned int /* main flash device with */
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define CONFIG_SYS_FLASH_ADDR0		0x555
#define CONFIG_SYS_FLASH_ADDR1		0x2AA
#define CONFIG_SYS_FLASH_2ND_16BIT_DEV	1	/* NIOS flash is a 16bit device */
#define CONFIG_SYS_MAX_FLASH_SECT	128
#else
/* use CFI flash driver */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_BOOTCS_START }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#endif

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_SIZE		0x04000000 /* 64 MByte */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#if defined (CONFIG_CAM5200)
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00040000)
#elif defined(CONFIG_TQM5200_B)
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00080000)
#else
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00060000)
#endif

/* Dynamic MTD partition support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=TQM5200-0"

#ifdef CONFIG_STK52XX
# if defined(CONFIG_TQM5200_B)
#  if defined(CONFIG_SYS_LOWBOOT)
#   define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:1m(firmware),"	\
						"256k(dtb),"		\
						"2304k(kernel),"	\
						"2560k(small-fs),"	\
						"2m(initrd),"		\
						"8m(misc),"		\
						"16m(big-fs)"
#  else	/* highboot */
#   define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:2560k(kernel),"	\
						"3584k(small-fs),"	\
						"2m(initrd),"		\
						"8m(misc),"		\
						"15m(big-fs),"		\
						"1m(firmware)"
#  endif /* CONFIG_SYS_LOWBOOT */
# else	/* !CONFIG_TQM5200_B */
#   define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:640k(firmware),"	\
						"128k(dtb),"		\
						"2304k(kernel),"	\
						"2m(initrd),"		\
						"4m(small-fs),"		\
						"8m(misc),"		\
						"15m(big-fs)"
# endif /* CONFIG_TQM5200_B */
#elif defined (CONFIG_CAM5200)
#   define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:768k(firmware),"	\
						"1792k(kernel),"	\
						"5632k(rootfs),"	\
						"24m(home)"
#elif defined (CONFIG_FO300)
#   define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:640k(firmware),"	\
						"1408k(kernel),"	\
						"2m(initrd),"		\
						"4m(small-fs),"		\
						"8m(misc),"		\
						"16m(big-fs)"
#else
# error "Unknown Carrier Board"
#endif	/* CONFIG_STK52XX */

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x4000	/* 16 k - keep small for fast booting */
#if defined(CONFIG_TQM5200_B) || defined (CONFIG_CAM5200)
#define CONFIG_ENV_SECT_SIZE	0x40000
#else
#define CONFIG_ENV_SECT_SIZE	0x20000
#endif /* CONFIG_TQM5200_B */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use ON-Chip SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
#define CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif


#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#if defined (CONFIG_CAM5200)
# define CONFIG_SYS_MONITOR_LEN	(256 << 10)	/* Reserve 256 kB for Monitor	*/
#elif defined(CONFIG_TQM5200_B)
# define CONFIG_SYS_MONITOR_LEN	(512 << 10)	/* Reserve 512 kB for Monitor	*/
#else
# define CONFIG_SYS_MONITOR_LEN	(384 << 10)	/* Reserve 384 kB for Monitor	*/
#endif

#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)	/* Reserve 1024 kB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
/*
 * Define CONFIG_MPC5xxx_FEC_MII10 to force FEC at 10Mb
 */
/* #define CONFIG_MPC5xxx_FEC_MII10 */
#define CONFIG_PHY_ADDR		0x00

/*
 * GPIO configuration
 *
 * use CS1: Bit 0 (mask: 0x80000000):
 *	   1 -> Pin gpio_wkup_6 as second SDRAM chip select (mem_cs1).
 * use ALT CAN position: Bits 2-3 (mask: 0x30000000):
 *	  00 -> No Alternatives, CAN1/2 on PSC2 according to PSC2 setting.
 *		SPI on PSC3 according to PSC3 setting. Use for CAM5200.
 *	  01 -> CAN1 on I2C1, CAN2 on Tmr0/1.
 *		Use for REV200 STK52XX boards and FO300 boards. Do not use
 *		with REV100 modules (because, there I2C1 is used as I2C bus).
 * use ATA: Bits 6-7 (mask 0x03000000):
 *	  00 -> No ATA chip selects, csb_4/5 used as normal chip selects.
 *		Use for CAM5200 board.
 *	  01 -> ATA cs0/1 on csb_4/5. Use for the remaining boards.
 * use PSC6: Bits 9-11 (mask 0x00700000):
 *	 000 -> use PSC6_0 to PSC6_3 as GPIO, PSC6 could not be used as
 *		UART, CODEC or IrDA.
 *		GPIO on PSC6_3 is used in post_hotkeys_pressed() to
 *		enable extended POST tests.
 *		Use for MINI-FAP and TQM5200_IB boards.
 *	 101 -> use PSC6 as UART. Pins PSC6_0 to PSC6_3 are used.
 *		Extended POST test is not available.
 *		Use for STK52xx, FO300 and CAM5200 boards.
 *		WARNING: When the extended POST is enabled, these bits will
 *			 be overridden by this code as GPIOs!
 * use PCI_DIS: Bit 16 (mask 0x00008000):
 *	   1 -> disable PCI controller (on CAM5200 board).
 * use USB: Bits 18-19 (mask 0x00003000):
 *	  10 -> two UARTs (on FO300 and CAM5200).
 * use PSC3: Bits 20-23 (mask: 0x00000f00):
 *	0000 -> All PSC3 pins are GPIOs.
 *	1100 -> UART/SPI (on FO300 board).
 *	0100 -> UART (on CAM5200 board).
 * use PSC2: Bits 25:27 (mask: 0x00000030):
 *	 000 -> All PSC2 pins are GPIOs.
 *	 100 -> UART (on CAM5200 board).
 *	 001 -> CAN1/2 on PSC2 pins.
 *		Use for REV100 STK52xx boards
 *	 01x -> Use AC97 (on FO300 board).
 * use PSC1: Bits 29-31 (mask: 0x00000007):
 *	 100 -> UART (on all boards).
 */
#if defined (CONFIG_MINIFAP)
# define CONFIG_SYS_GPS_PORT_CONFIG	0x91000004
#elif defined (CONFIG_STK52XX)
# if defined (CONFIG_STK52XX_REV100)
#  define CONFIG_SYS_GPS_PORT_CONFIG	0x81500014
# else /* STK52xx REV200 and above */
#  if defined (CONFIG_TQM5200_REV100)
#   error TQM5200 REV100 not supported on STK52XX REV200 or above
#  else/* TQM5200 REV200 and above */
#   define CONFIG_SYS_GPS_PORT_CONFIG	0x91500404
#  endif
# endif
#elif defined (CONFIG_FO300)
# define CONFIG_SYS_GPS_PORT_CONFIG	0x91502c24
#elif defined (CONFIG_CAM5200)
# define CONFIG_SYS_GPS_PORT_CONFIG	0x8050A444
#else  /* TMQ5200 Inbetriebnahme-Board */
# define CONFIG_SYS_GPS_PORT_CONFIG	0x81000004
#endif

/*
 * RTC configuration
 */
#if defined (CONFIG_STK52XX) && !defined (CONFIG_STK52XX_REV100)
# define CONFIG_RTC_M41T11 1
# define CONFIG_SYS_I2C_RTC_ADDR 0x68
# define CONFIG_SYS_M41T11_BASE_YEAR	1900    /* because Linux uses the same base
					   year */
#else
# define CONFIG_RTC_MPC5200	1	/* use internal MPC5200 RTC */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define	CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

/* Enable an alternate, more extensive memory test */
#define CONFIG_SYS_ALT_MEMTEST

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Enable loopw command.
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#ifdef CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2
#define CONFIG_SYS_BOOTCS_CFG		0x0008DF30 /* for pci_clk  = 66 MHz */
#else
#define CONFIG_SYS_BOOTCS_CFG		0x0004DF30 /* for pci_clk = 33 MHz */
#endif
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_LAST_STAGE_INIT

/*
 * SRAM - Do not map below 2 GB in address space, because this area is used
 * for SDRAM autosizing.
 */
#define CONFIG_SYS_CS2_START		0xE5000000
#define CONFIG_SYS_CS2_SIZE		0x100000	/* 1 MByte */
#define CONFIG_SYS_CS2_CFG		0x0004D930

/*
 * Grafic controller - Do not map below 2 GB in address space, because this
 * area is used for SDRAM autosizing.
 */
#define SM501_FB_BASE		0xE0000000
#define CONFIG_SYS_CS1_START		(SM501_FB_BASE)
#define CONFIG_SYS_CS1_SIZE		0x4000000	/* 64 MByte */
#define CONFIG_SYS_CS1_CFG		0x8F48FF70
#define SM501_MMIO_BASE		CONFIG_SYS_CS1_START + 0x03E00000

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333311	/* 1 dead cycle for flash and SM501 */

#if defined(CONFIG_CAM5200)
#define CONFIG_SYS_CS4_START		0xB0000000
#define CONFIG_SYS_CS4_SIZE		0x00010000
#define CONFIG_SYS_CS4_CFG		0x01019C10

#define CONFIG_SYS_CS5_START		0xD0000000
#define CONFIG_SYS_CS5_SIZE		0x01208000
#define CONFIG_SYS_CS5_CFG		0x1414BF10
#endif

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef	CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/

#define CONFIG_IDE_RESET		/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 2 drives per IDE bus	*/

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

/* Support ATAPI devices */
#define CONFIG_ATAPI			1

/*-----------------------------------------------------------------------
 * Open firmware flat tree support
 *-----------------------------------------------------------------------
 */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

#endif /* __CONFIG_H */
