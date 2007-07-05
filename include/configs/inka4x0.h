/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#define CONFIG_INKA4X0		1	/* INKA4x0 board			*/

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz		*/

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define CONFIG_MISC_INIT_F	1	/* Use misc_init_f()			*/

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1	*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps	*/
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

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

#define CFG_XLB_PIPELINING	1

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB


#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (TEXT_BASE == 0xFFE00000)		/* Boot low */
#   define CFG_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_ETHADDR		00:a0:a4:03:00:00
#define	CONFIG_OVERWRITE_ETHADDR_ONCE

#define	CONFIG_IPADDR		192.168.100.2
#define	CONFIG_SERVERIP		192.168.100.1
#define	CONFIG_NETMASK		255.255.255.0
#define HOSTNAME		inka4x0
#define CONFIG_BOOTFILE		/tftpboot/inka4x0/uImage
#define	CONFIG_ROOTPATH		/opt/eldk/ppc_6xx

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate}\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"enable_disp=mw.l 100000 04000000 1;"				\
		"cp.l 100000 f0000b20 1;"				\
		"cp.l 100000 f0000b28 1\0"				\
	"ideargs=setenv bootargs root=/dev/hda1 rw\0"			\
	"ide_boot=ext2load ide 0:1 200000 uImage;"			\
		"run ideargs addip addcons enable_disp;bootm"		\
	"brightness=255\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run ide_boot"

/*
 * IPB Bus clocking configuration.
 */
#define CFG_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * Flash configuration
 */
#define CFG_FLASH_BASE		0xFFE00000

#define CFG_FLASH_SIZE		0x00200000 /* 2 MByte */
#define CFG_MAX_FLASH_SECT	35	/* max num of sects on one chip */

#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x4000) /* second sector */
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x2000
#define CFG_ENV_SECT_SIZE	0x2000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

#define CONFIG_MPC5200_DDR

/* Use ON-Chip SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
/*
 * Define CONFIG_FEC_10MBIT to force FEC at 10Mb
 */
/* #define CONFIG_FEC_10MBIT 1 */
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_MII

/*
 * GPIO configuration
 *
 * use CS1 as gpio_wkup_6 output
 *	Bit 0 (mask: 0x80000000): 0
 * use ALT CAN position: Bits 2-3 (mask: 0x30000000):
 *	00 -> No Alternatives, I2C1 is used for onboard EEPROM
 *	01 -> CAN1 on I2C1, CAN2 on Tmr0/1 do not use on TQM5200 with onboard
 *	      EEPROM
 * use PSC1 as UART: Bits 28-31 (mask: 0x00000007): 0100
 * use PSC6_1 and PSC6_3 as GPIO: Bits 9:11 (mask: 0x07000000):
 *	011 -> PSC6 could not be used as UART or CODEC. IrDA still possible.
 */
#define CFG_GPS_PORT_CONFIG	0x01001004

/*
 * RTC configuration
 */
#define CONFIG_RTC_MPC5200	1	/* use internal MPC5200 RTC */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/* Enable an alternate, more extensive memory test */
#define CFG_ALT_MEMTEST

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Enable loopw commando. This has only affect, if CFG_CMD_MEM is defined,
 * which is normally part of the default commands (CFV_CMD_DFL)
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE
#else
#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		0
#endif

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00087800 /* for pci_clk  = 66 MHz */
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

/* 32Mbit SRAM @0x30000000 */
#define CFG_CS1_START		0x30000000
#define CFG_CS1_SIZE		0x00400000
#define CFG_CS1_CFG		0x31800 /* for pci_clk = 33 MHz */

/* 2 quad UART @0x80000000 (MBAR is relocated to 0xF0000000) */
#define CFG_CS2_START		0x80000000
#define CFG_CS2_SIZE		0x0001000
#define CFG_CS2_CFG		0x21800  /* for pci_clk = 33 MHz */

/* GPIO in @0x30400000 */
#define CFG_CS3_START		0x30400000
#define CFG_CS3_SIZE		0x00100000
#define CFG_CS3_CFG		0x31800 /* for pci_clk = 33 MHz */

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_OHCI
#define CONFIG_USB_CLOCK	0x00015555
#define CONFIG_USB_CONFIG	0x00001000
#define CONFIG_USB_STORAGE

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define	CONFIG_IDE_RESET		/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	2	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA
#define CFG_ATA_DATA_OFFSET	0x0060	/* Offset for data I/O		*/
#define CFG_ATA_REG_OFFSET (CFG_ATA_DATA_OFFSET) /* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0x005C	/* Offset for alternate registers */
#define CFG_ATA_STRIDE          4	/* Interval between registers	*/

#define CONFIG_ATAPI            1

#define CFG_BRIGHTNESS          0xFF	/* LCD Default Brightness (255 = off) */

#endif /* __CONFIG_H */
